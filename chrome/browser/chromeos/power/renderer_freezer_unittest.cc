// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/power/renderer_freezer.h"

#include <string>

#include "base/command_line.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "chrome/browser/chromeos/login/users/scoped_test_user_manager.h"
#include "chrome/browser/chromeos/settings/cros_settings.h"
#include "chrome/browser/chromeos/settings/device_settings_service.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/extensions/test_extension_system.h"
#include "chrome/test/base/testing_browser_process.h"
#include "chrome/test/base/testing_profile.h"
#include "chrome/test/base/testing_profile_manager.h"
#include "chromeos/dbus/dbus_thread_manager.h"
#include "chromeos/dbus/fake_power_manager_client.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/site_instance.h"
#include "content/public/test/mock_render_process_host.h"
#include "content/public/test/test_browser_thread_bundle.h"
#include "extensions/browser/notification_types.h"
#include "extensions/browser/process_manager.h"
#include "extensions/browser/process_map.h"
#include "extensions/common/extension_builder.h"
#include "extensions/common/manifest_handlers/background_info.h"
#include "extensions/common/value_builder.h"
#include "testing/gtest/include/gtest/gtest-death-test.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace chromeos {

namespace {
// Class that delegates used in testing can inherit from to record calls that
// are made by the code being tested.
class ActionRecorder {
 public:
  ActionRecorder() {}
  virtual ~ActionRecorder() {}

  // Returns a comma-separated string describing the actions that were
  // requested since the previous call to GetActions() (i.e. results are
  // non-repeatable).
  std::string GetActions() {
    std::string actions = actions_;
    actions_.clear();
    return actions;
  }

 protected:
  // Appends |new_action| to |actions_|, using a comma as a separator if
  // other actions are already listed.
  void AppendAction(const std::string& new_action) {
    if (!actions_.empty())
      actions_ += ",";
    actions_ += new_action;
  }

 private:
  // Comma-separated list of actions that have been performed.
  std::string actions_;

  DISALLOW_COPY_AND_ASSIGN(ActionRecorder);
};

// Actions that can be returned by TestDelegate::GetActions().
const char kSetShouldFreezeRenderer[] = "set_should_freeze_renderer";
const char kSetShouldNotFreezeRenderer[] = "set_should_not_freeze_renderer";
const char kFreezeRenderers[] = "freeze_renderers";
const char kThawRenderers[] = "thaw_renderers";
const char kNoActions[] = "";

// Test implementation of RendererFreezer::Delegate that records the actions it
// was asked to perform.
class TestDelegate : public RendererFreezer::Delegate, public ActionRecorder {
 public:
  TestDelegate()
      : can_freeze_renderers_(true),
        freeze_renderers_result_(true),
        thaw_renderers_result_(true) {}

  ~TestDelegate() override {}

  // RendererFreezer::Delegate overrides.
  void SetShouldFreezeRenderer(base::ProcessHandle handle,
                               bool frozen) override {
    AppendAction(frozen ? kSetShouldFreezeRenderer
                        : kSetShouldNotFreezeRenderer);
  }
  bool FreezeRenderers() override {
    AppendAction(kFreezeRenderers);

    return freeze_renderers_result_;
  }
  bool ThawRenderers() override {
    AppendAction(kThawRenderers);

    return thaw_renderers_result_;
  }
  bool CanFreezeRenderers() override { return can_freeze_renderers_; }

  void set_freeze_renderers_result(bool result) {
    freeze_renderers_result_ = result;
  }

  void set_thaw_renderers_result(bool result) {
    thaw_renderers_result_ = result;
  }

  // Sets whether the delegate is capable of freezing renderers.  This also
  // changes |freeze_renderers_result_| and |thaw_renderers_result_|.
  void set_can_freeze_renderers(bool can_freeze) {
    can_freeze_renderers_ = can_freeze;

    // If the delegate cannot freeze renderers, then the result of trying to do
    // so will be false.
    freeze_renderers_result_ = can_freeze;
    thaw_renderers_result_ = can_freeze;
  }

 private:
  bool can_freeze_renderers_;
  bool freeze_renderers_result_;
  bool thaw_renderers_result_;

  DISALLOW_COPY_AND_ASSIGN(TestDelegate);
};

}  // namespace

class RendererFreezerTest : public testing::Test {
 public:
  RendererFreezerTest()
      : power_manager_client_(new FakePowerManagerClient()),
        test_delegate_(new TestDelegate()) {
    DBusThreadManager::GetSetterForTesting()->SetPowerManagerClient(
        scoped_ptr<PowerManagerClient>(power_manager_client_));
  }

  ~RendererFreezerTest() override {
    renderer_freezer_.reset();

    DBusThreadManager::Shutdown();
  }

 protected:
  void Init() {
    renderer_freezer_.reset(new RendererFreezer(
        scoped_ptr<RendererFreezer::Delegate>(test_delegate_)));
  }

  // Owned by DBusThreadManager.
  FakePowerManagerClient* power_manager_client_;

  // Owned by |renderer_freezer_|.
  TestDelegate* test_delegate_;
  scoped_ptr<RendererFreezer> renderer_freezer_;

 private:
  content::TestBrowserThreadBundle browser_thread_bundle_;

  DISALLOW_COPY_AND_ASSIGN(RendererFreezerTest);
};

// Tests that the RendererFreezer freezes renderers on suspend and thaws them on
// resume.
TEST_F(RendererFreezerTest, SuspendResume) {
  Init();

  power_manager_client_->SendSuspendImminent();

  // The RendererFreezer should have grabbed an asynchronous callback and done
  // nothing else.
  EXPECT_EQ(1, power_manager_client_->GetNumPendingSuspendReadinessCallbacks());
  EXPECT_EQ(kNoActions, test_delegate_->GetActions());

  // The RendererFreezer should eventually freeze the renderers and run the
  // callback.
  base::RunLoop().RunUntilIdle();
  EXPECT_EQ(0, power_manager_client_->GetNumPendingSuspendReadinessCallbacks());
  EXPECT_EQ(kFreezeRenderers, test_delegate_->GetActions());

  // The renderers should be thawed when we resume.
  power_manager_client_->SendSuspendDone();
  EXPECT_EQ(kThawRenderers, test_delegate_->GetActions());
}

// Tests that the RendereFreezer doesn't freeze renderers if the suspend attempt
// was canceled before it had a chance to complete.
TEST_F(RendererFreezerTest, SuspendCanceled) {
  Init();

  // We shouldn't do anything yet.
  power_manager_client_->SendSuspendImminent();
  EXPECT_EQ(kNoActions, test_delegate_->GetActions());

  // If a suspend gets canceled for any reason, we should see a SuspendDone().
  power_manager_client_->SendSuspendDone();

  // We shouldn't try to freeze the renderers now.
  base::RunLoop().RunUntilIdle();
  EXPECT_EQ(kNoActions, test_delegate_->GetActions());
}

// Tests that the renderer freezer does nothing if the delegate cannot freeze
// renderers.
TEST_F(RendererFreezerTest, DelegateCannotFreezeRenderers) {
  test_delegate_->set_can_freeze_renderers(false);
  Init();

  power_manager_client_->SendSuspendImminent();

  // The RendererFreezer should not have grabbed a callback or done anything
  // else.
  EXPECT_EQ(0, power_manager_client_->GetNumPendingSuspendReadinessCallbacks());
  EXPECT_EQ(kNoActions, test_delegate_->GetActions());

  // There should be nothing in the message loop.
  base::RunLoop().RunUntilIdle();
  EXPECT_EQ(kNoActions, test_delegate_->GetActions());

  // Nothing happens on resume.
  power_manager_client_->SendSuspendDone();
  EXPECT_EQ(kNoActions, test_delegate_->GetActions());
}

// Tests that the RendererFreezer does nothing on resume if the freezing
// operation was unsuccessful.
TEST_F(RendererFreezerTest, ErrorFreezingRenderers) {
  Init();
  test_delegate_->set_freeze_renderers_result(false);

  power_manager_client_->SendSuspendImminent();
  EXPECT_EQ(1, power_manager_client_->GetNumPendingSuspendReadinessCallbacks());

  // The freezing operation should fail, but we should still report readiness.
  base::RunLoop().RunUntilIdle();
  EXPECT_EQ(kFreezeRenderers, test_delegate_->GetActions());
  EXPECT_EQ(0, power_manager_client_->GetNumPendingSuspendReadinessCallbacks());

  // Since the delegate reported that the freezing was unsuccessful, don't do
  // anything on resume.
  power_manager_client_->SendSuspendDone();
  EXPECT_EQ(kNoActions, test_delegate_->GetActions());
}

#if defined(GTEST_HAS_DEATH_TEST)
// Tests that the RendererFreezer crashes the browser if the freezing operation
// was successful but the thawing operation failed.
TEST_F(RendererFreezerTest, ErrorThawingRenderers) {
  // The "threadsafe" style of death test re-executes the unit test binary,
  // which in turn re-initializes some global state leading to failed CHECKs.
  // Instead, we use the "fast" style here to prevent re-initialization.
  ::testing::FLAGS_gtest_death_test_style = "fast";
  Init();
  test_delegate_->set_thaw_renderers_result(false);

  power_manager_client_->SendSuspendImminent();
  base::RunLoop().RunUntilIdle();
  EXPECT_EQ(kFreezeRenderers, test_delegate_->GetActions());

  EXPECT_DEATH(power_manager_client_->SendSuspendDone(), "Unable to thaw");
}
#endif  // GTEST_HAS_DEATH_TEST

class RendererFreezerTestWithExtensions : public RendererFreezerTest {
 public:
  RendererFreezerTestWithExtensions() {}
  ~RendererFreezerTestWithExtensions() override {}

  // testing::Test overrides.
  void SetUp() override {
    RendererFreezerTest::SetUp();

    profile_manager_.reset(
        new TestingProfileManager(TestingBrowserProcess::GetGlobal()));

    // Must be called from testing::Test::SetUp.
    EXPECT_TRUE(profile_manager_->SetUp());

    profile_ = profile_manager_->CreateTestingProfile("RendererFreezerTest");
  }
  void TearDown() override {
    profile_ = NULL;

    profile_manager_->DeleteAllTestingProfiles();

    base::RunLoop().RunUntilIdle();

    profile_manager_.reset();

    RendererFreezerTest::TearDown();
  }

 protected:
  void Init() {
    RendererFreezerTest::Init();

    extensions::TestExtensionSystem* extension_system =
        static_cast<extensions::TestExtensionSystem*>(
            extensions::ExtensionSystem::Get(profile_));
    extension_system->CreateExtensionService(
        base::CommandLine::ForCurrentProcess(),
        base::FilePath()  /* install_directory */,
        false  /* autoupdate_enabled*/);
  }

  void CreateRenderProcessForExtension(extensions::Extension* extension) {
    scoped_ptr<content::MockRenderProcessHostFactory> rph_factory(
        new content::MockRenderProcessHostFactory());
    content::SiteInstance* site_instance =
        extensions::ProcessManager::Get(profile_)->GetSiteInstanceForURL(
            extensions::BackgroundInfo::GetBackgroundURL(extension));
    content::RenderProcessHost* rph =
        rph_factory->CreateRenderProcessHost(profile_, site_instance);

    // Fake that the RenderProcessHost is hosting the gcm app.
    extensions::ProcessMap::Get(profile_)
        ->Insert(extension->id(), rph->GetID(), site_instance->GetId());

    // Send the notification that the RenderProcessHost has been created.
    content::NotificationService::current()->Notify(
        content::NOTIFICATION_RENDERER_PROCESS_CREATED,
        content::Source<content::RenderProcessHost>(rph),
        content::NotificationService::NoDetails());
  }

  // Owned by |profile_manager_|.
  TestingProfile* profile_;
  scoped_ptr<TestingProfileManager> profile_manager_;

 private:
  // Chrome OS needs extra services to run in the following order.
  chromeos::ScopedTestDeviceSettingsService test_device_settings_service_;
  chromeos::ScopedTestCrosSettings test_cros_settings_;
  chromeos::ScopedTestUserManager test_user_manager_;

  DISALLOW_COPY_AND_ASSIGN(RendererFreezerTestWithExtensions);
};

// Tests that the RendererFreezer freezes renderers that are not hosting
// GCM extensions.
TEST_F(RendererFreezerTestWithExtensions, FreezesNonExtensionRenderers) {
  Init();

  // Create the mock RenderProcessHost.
  scoped_ptr<content::MockRenderProcessHostFactory> rph_factory(
      new content::MockRenderProcessHostFactory());
  content::RenderProcessHost* rph = rph_factory->CreateRenderProcessHost(
      profile_, content::SiteInstance::Create(profile_));

  // Send the notification that the RenderProcessHost has been created.
  content::NotificationService::current()->Notify(
      content::NOTIFICATION_RENDERER_PROCESS_CREATED,
      content::Source<content::RenderProcessHost>(rph),
      content::NotificationService::NoDetails());

  EXPECT_EQ(kSetShouldFreezeRenderer, test_delegate_->GetActions());
}

// Tests that the RendererFreezer does not freeze renderers that are hosting
// extensions that use GCM.
TEST_F(RendererFreezerTestWithExtensions, DoesNotFreezeGcmExtensionRenderers) {
  Init();

  // First build the GCM extension.
  scoped_refptr<extensions::Extension> gcm_app =
      extensions::ExtensionBuilder()
          .SetManifest(extensions::DictionaryBuilder()
                       .Set("name", "GCM App")
                       .Set("version", "1.0.0")
                       .Set("manifest_version", 2)
                       .Set("app",
                            extensions::DictionaryBuilder()
                            .Set("background",
                                 extensions::DictionaryBuilder()
                                 .Set("scripts",
                                      extensions::ListBuilder()
                                      .Append("background.js"))))
                       .Set("permissions",
                            extensions::ListBuilder()
                            .Append("gcm")))
          .Build();

  // Now install it and give it a renderer.
  extensions::ExtensionSystem::Get(profile_)
      ->extension_service()
      ->AddExtension(gcm_app.get());
  CreateRenderProcessForExtension(gcm_app.get());

  EXPECT_EQ(kSetShouldNotFreezeRenderer, test_delegate_->GetActions());
}

// Tests that the RendererFreezer freezes renderers that are hosting extensions
// that do not use GCM.
TEST_F(RendererFreezerTestWithExtensions, FreezesNonGcmExtensionRenderers) {
  Init();

  // First build the extension.
  scoped_refptr<extensions::Extension> background_app =
      extensions::ExtensionBuilder()
          .SetManifest(extensions::DictionaryBuilder()
                       .Set("name", "Background App")
                       .Set("version", "1.0.0")
                       .Set("manifest_version", 2)
                       .Set("app",
                            extensions::DictionaryBuilder()
                            .Set("background",
                                 extensions::DictionaryBuilder()
                                 .Set("scripts",
                                      extensions::ListBuilder()
                                      .Append("background.js")))))
          .Build();

  // Now install it and give it a renderer.
  extensions::ExtensionSystem::Get(profile_)
      ->extension_service()
      ->AddExtension(background_app.get());
  CreateRenderProcessForExtension(background_app.get());

  EXPECT_EQ(kSetShouldFreezeRenderer, test_delegate_->GetActions());
}

}  // namespace chromeos
