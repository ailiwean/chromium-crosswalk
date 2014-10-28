// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/power/renderer_freezer.h"

#include <string>

#include "base/bind.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/process/process_handle.h"
#include "chromeos/dbus/dbus_thread_manager.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/render_process_host.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/notification_types.h"
#include "extensions/browser/process_map.h"
#include "extensions/common/extension.h"
#include "extensions/common/permissions/api_permission.h"
#include "extensions/common/permissions/permissions_data.h"

namespace chromeos {

RendererFreezer::RendererFreezer(scoped_ptr<RendererFreezer::Delegate> delegate)
    : frozen_(false), delegate_(delegate.Pass()), weak_factory_(this) {
  if (delegate_->CanFreezeRenderers()) {
    DBusThreadManager::Get()->GetPowerManagerClient()->AddObserver(this);
    registrar_.Add(
        this,
        content::NOTIFICATION_RENDERER_PROCESS_CREATED,
        content::NotificationService::AllBrowserContextsAndSources());
  }
}

RendererFreezer::~RendererFreezer() {
  if (delegate_->CanFreezeRenderers())
    DBusThreadManager::Get()->GetPowerManagerClient()->RemoveObserver(this);

  for (int rph_id : gcm_extension_processes_) {
    content::RenderProcessHost* host =
        content::RenderProcessHost::FromID(rph_id);
    if (host)
      host->RemoveObserver(this);
  }
}

void RendererFreezer::SuspendImminent() {
  // If there was already a callback pending, this will cancel it and create a
  // new one.
  suspend_readiness_callback_.Reset(
      base::Bind(&RendererFreezer::OnReadyToSuspend,
                 weak_factory_.GetWeakPtr(),
                 DBusThreadManager::Get()
                     ->GetPowerManagerClient()
                     ->GetSuspendReadinessCallback()));

  base::MessageLoop::current()->PostTask(
      FROM_HERE, suspend_readiness_callback_.callback());
}

void RendererFreezer::SuspendDone(const base::TimeDelta& sleep_duration) {
  // If we get a SuspendDone before we've had a chance to run OnReadyForSuspend,
  // we should cancel it because we no longer want to freeze the renderers.  If
  // we've already run it then cancelling the callback shouldn't really make a
  // difference.
  suspend_readiness_callback_.Cancel();

  if (!frozen_)
    return;

  if (!delegate_->ThawRenderers()) {
    // We failed to write the thaw command and the renderers are still frozen.
    // We are in big trouble because none of the tabs will be responsive so
    // let's crash the browser instead.
    LOG(FATAL) << "Unable to thaw renderers.";
  }

  frozen_ = false;
}

void RendererFreezer::Observe(int type,
                              const content::NotificationSource& source,
                              const content::NotificationDetails& details) {
  switch (type) {
    case content::NOTIFICATION_RENDERER_PROCESS_CREATED: {
      content::RenderProcessHost* process =
          content::Source<content::RenderProcessHost>(source).ptr();
      OnRenderProcessCreated(process);
      break;
    }
    default: {
      NOTREACHED();
      break;
    }
  }
}

void RendererFreezer::RenderProcessExited(content::RenderProcessHost* host,
                                          base::TerminationStatus status,
                                          int exit_code) {
  auto it = gcm_extension_processes_.find(host->GetID());
  if (it == gcm_extension_processes_.end()) {
    LOG(ERROR) << "Received unrequested RenderProcessExited message";
    return;
  }
  gcm_extension_processes_.erase(it);

  // When this function is called, the renderer process has died but the
  // RenderProcessHost will not be destroyed.  If a new renderer process is
  // created for this RPH, registering as an observer again will trigger a
  // warning about duplicate observers.  To prevent this we just stop observing
  // this RPH until another renderer process is created for it.
  host->RemoveObserver(this);
}

void RendererFreezer::RenderProcessHostDestroyed(
    content::RenderProcessHost* host) {
  auto it = gcm_extension_processes_.find(host->GetID());
  if (it == gcm_extension_processes_.end()) {
    LOG(ERROR) << "Received unrequested RenderProcessHostDestroyed message";
    return;
  }

  gcm_extension_processes_.erase(it);
}

void RendererFreezer::OnRenderProcessCreated(content::RenderProcessHost* rph) {
  const int rph_id = rph->GetID();

  if (gcm_extension_processes_.find(rph_id) != gcm_extension_processes_.end()) {
    LOG(ERROR) << "Received duplicate notifications about the creation of a "
               << "RenderProcessHost with id " << rph_id;
    return;
  }

  // According to extensions::ProcessMap, extensions and renderers have a
  // many-to-many relationship.  Specifically, a hosted app can appear in many
  // renderers while any other kind of extension can be running in "split mode"
  // if there is an incognito window open and so could appear in two renderers.
  //
  // We don't care about hosted apps because they cannot use GCM so we only need
  // to worry about extensions in "split mode".  Luckily for us this function is
  // called any time a new renderer process is created so we don't really need
  // to care whether we are currently in an incognito context.  We just need to
  // iterate over all the extensions in the newly created process and take the
  // appropriate action based on whether we find an extension using GCM.
  content::BrowserContext* context = rph->GetBrowserContext();
  extensions::ExtensionRegistry* registry =
      extensions::ExtensionRegistry::Get(context);
  for (const std::string& extension_id :
       extensions::ProcessMap::Get(context)->GetExtensionsInProcess(rph_id)) {
    if (!registry->GetExtensionById(extension_id,
                                    extensions::ExtensionRegistry::ENABLED)
             ->permissions_data()
             ->HasAPIPermission(extensions::APIPermission::kGcm)) {
      continue;
    }

    // This renderer has an extension that is using GCM.  Make sure it is not
    // frozen during suspend.
    delegate_->SetShouldFreezeRenderer(rph->GetHandle(), false);
    gcm_extension_processes_.insert(rph_id);

    // Watch to see if the renderer process or the RenderProcessHost is
    // destroyed.
    rph->AddObserver(this);
    return;
  }

  // We didn't find an extension in this RenderProcessHost that is using GCM so
  // we can go ahead and freeze it on suspend.
  delegate_->SetShouldFreezeRenderer(rph->GetHandle(), true);
}

void RendererFreezer::OnReadyToSuspend(
    const base::Closure& power_manager_callback) {
  if (delegate_->FreezeRenderers())
    frozen_ = true;

  DCHECK(!power_manager_callback.is_null());
  power_manager_callback.Run();
}

}  // namespace chromeos
