// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_BROWSER_SHELL_NETWORK_DELEGATE_H_
#define CONTENT_SHELL_BROWSER_SHELL_NETWORK_DELEGATE_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "net/base/network_delegate.h"

namespace content {

class ShellNetworkDelegate : public net::NetworkDelegate {
 public:
  ShellNetworkDelegate();
  virtual ~ShellNetworkDelegate();

  static void SetAcceptAllCookies(bool accept);

 private:
  // net::NetworkDelegate implementation.
  virtual int OnBeforeURLRequest(net::URLRequest* request,
                                 const net::CompletionCallback& callback,
                                 GURL* new_url) override;
  virtual int OnBeforeSendHeaders(net::URLRequest* request,
                                  const net::CompletionCallback& callback,
                                  net::HttpRequestHeaders* headers) override;
  virtual void OnSendHeaders(net::URLRequest* request,
                             const net::HttpRequestHeaders& headers) override;
  virtual int OnHeadersReceived(
      net::URLRequest* request,
      const net::CompletionCallback& callback,
      const net::HttpResponseHeaders* original_response_headers,
      scoped_refptr<net::HttpResponseHeaders>* override_response_headers,
      GURL* allowed_unsafe_redirect_url) override;
  virtual void OnBeforeRedirect(net::URLRequest* request,
                                const GURL& new_location) override;
  virtual void OnResponseStarted(net::URLRequest* request) override;
  virtual void OnRawBytesRead(const net::URLRequest& request,
                              int bytes_read) override;
  virtual void OnCompleted(net::URLRequest* request, bool started) override;
  virtual void OnURLRequestDestroyed(net::URLRequest* request) override;
  virtual void OnPACScriptError(int line_number,
                                const base::string16& error) override;
  virtual AuthRequiredResponse OnAuthRequired(
      net::URLRequest* request,
      const net::AuthChallengeInfo& auth_info,
      const AuthCallback& callback,
      net::AuthCredentials* credentials) override;
  virtual bool OnCanGetCookies(const net::URLRequest& request,
                               const net::CookieList& cookie_list) override;
  virtual bool OnCanSetCookie(const net::URLRequest& request,
                              const std::string& cookie_line,
                              net::CookieOptions* options) override;
  virtual bool OnCanAccessFile(const net::URLRequest& request,
                               const base::FilePath& path) const override;
  virtual bool OnCanThrottleRequest(
      const net::URLRequest& request) const override;
  virtual int OnBeforeSocketStreamConnect(
      net::SocketStream* stream,
      const net::CompletionCallback& callback) override;

  DISALLOW_COPY_AND_ASSIGN(ShellNetworkDelegate);
};

}  // namespace content

#endif  // CONTENT_SHELL_BROWSER_SHELL_NETWORK_DELEGATE_H_
