// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SIGNIN_CORE_BROWSER_CHILD_ACCOUNT_INFO_FETCHER_H_
#define COMPONENTS_SIGNIN_CORE_BROWSER_CHILD_ACCOUNT_INFO_FETCHER_H_

#include "base/threading/thread_checker.h"
#include "base/timer/timer.h"
#include "components/invalidation/public/invalidation_handler.h"
#include "google_apis/gaia/gaia_auth_consumer.h"
#include "google_apis/gaia/oauth2_token_service.h"
#include "net/base/backoff_entry.h"

namespace net {
class URLRequestContextGetter;
}
namespace invalidation {
class InvalidationService;
}
class AccountFetcherService;
class GaiaAuthFetcher;

class ChildAccountInfoFetcher : public OAuth2TokenService::Consumer,
                                public GaiaAuthConsumer,
                                public syncer::InvalidationHandler {
 public:
  ChildAccountInfoFetcher(
      OAuth2TokenService* token_service,
      net::URLRequestContextGetter* request_context_getter,
      AccountFetcherService* fetcher_service,
      invalidation::InvalidationService* invalidation_service,
      const std::string& account_id);
  ~ChildAccountInfoFetcher() override;

 private:
  void FetchIfNotInProgress();
  void HandleFailure();

  // OAuth2TokenService::Consumer:
  void OnGetTokenSuccess(const OAuth2TokenService::Request* request,
                         const std::string& access_token,
                         const base::Time& expiration_time) override;
  void OnGetTokenFailure(const OAuth2TokenService::Request* request,
                         const GoogleServiceAuthError& error) override;

  // GaiaAuthConsumer:
  void OnClientLoginSuccess(const ClientLoginResult& result) override;
  void OnClientLoginFailure(const GoogleServiceAuthError& error) override;
  void OnGetUserInfoSuccess(const UserInfoMap& data) override;
  void OnGetUserInfoFailure(const GoogleServiceAuthError& error) override;

  // syncer::InvalidationHandler:
  void OnInvalidatorStateChange(syncer::InvalidatorState state) override;
  void OnIncomingInvalidation(
      const syncer::ObjectIdInvalidationMap& invalidation_map) override;
  std::string GetOwnerName() const override;

  OAuth2TokenService* token_service_;
  net::URLRequestContextGetter* request_context_getter_;
  AccountFetcherService* fetcher_service_;
  invalidation::InvalidationService* invalidation_service_;
  const std::string account_id_;

  // If fetching fails, retry with exponential backoff.
  base::OneShotTimer<ChildAccountInfoFetcher> timer_;
  net::BackoffEntry backoff_;

  scoped_ptr<OAuth2TokenService::Request> login_token_request_;
  scoped_ptr<GaiaAuthFetcher> gaia_auth_fetcher_;

  bool fetch_in_progress_;
  base::ThreadChecker thread_checker_;

  DISALLOW_COPY_AND_ASSIGN(ChildAccountInfoFetcher);
};

#endif  // COMPONENTS_SIGNIN_CORE_BROWSER_CHILD_ACCOUNT_INFO_FETCHER_H_
