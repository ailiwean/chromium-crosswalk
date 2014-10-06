// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_BROWSER_SHELL_DEVTOOLS_FRONTEND_H_
#define CONTENT_SHELL_BROWSER_SHELL_DEVTOOLS_FRONTEND_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/devtools_frontend_host.h"
#include "content/public/browser/web_contents_observer.h"

namespace content {

GURL GetDevToolsPathAsURL(const std::string& settings,
                          const std::string& frontend_url);

class RenderViewHost;
class Shell;
class WebContents;

class ShellDevToolsFrontend : public WebContentsObserver,
                              public DevToolsFrontendHost::Delegate,
                              public DevToolsAgentHostClient {
 public:
  static ShellDevToolsFrontend* Show(WebContents* inspected_contents);
  static ShellDevToolsFrontend* Show(WebContents* inspected_contents,
                                     const std::string& settings,
                                     const std::string& frontend_url);
  void Activate();
  void Focus();
  void InspectElementAt(int x, int y);
  void Close();

  Shell* frontend_shell() const { return frontend_shell_; }

 private:
  ShellDevToolsFrontend(Shell* frontend_shell, DevToolsAgentHost* agent_host);
  virtual ~ShellDevToolsFrontend();

  // WebContentsObserver overrides
  virtual void RenderViewCreated(RenderViewHost* render_view_host) override;
  virtual void DocumentOnLoadCompletedInMainFrame() override;
  virtual void WebContentsDestroyed() override;
  virtual void RenderProcessGone(base::TerminationStatus status) override;

  // content::DevToolsFrontendHost::Delegate implementation.
  virtual void HandleMessageFromDevToolsFrontend(
      const std::string& message) override;
  virtual void HandleMessageFromDevToolsFrontendToBackend(
      const std::string& message) override;

  // content::DevToolsAgentHostClient implementation.
  virtual void DispatchProtocolMessage(
      DevToolsAgentHost* agent_host, const std::string& message) override;
  virtual void AgentHostClosed(
      DevToolsAgentHost* agent_host, bool replaced) override;

  Shell* frontend_shell_;
  scoped_refptr<DevToolsAgentHost> agent_host_;
  scoped_ptr<DevToolsFrontendHost> frontend_host_;

  DISALLOW_COPY_AND_ASSIGN(ShellDevToolsFrontend);
};

}  // namespace content

#endif  // CONTENT_SHELL_BROWSER_SHELL_DEVTOOLS_FRONTEND_H_
