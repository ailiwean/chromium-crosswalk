// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_LOGIN_SIGNIN_OAUTH2_LOGIN_MANAGER_H_
#define CHROME_BROWSER_CHROMEOS_LOGIN_SIGNIN_OAUTH2_LOGIN_MANAGER_H_

#include <string>

#include "base/memory/scoped_ptr.h"
#include "base/observer_list.h"
#include "base/time/time.h"
#include "chrome/browser/chromeos/login/signin/oauth2_login_verifier.h"
#include "chrome/browser/chromeos/login/signin/oauth2_token_fetcher.h"
#include "chrome/browser/chromeos/login/signin/token_handle_util.h"
#include "components/keyed_service/core/keyed_service.h"
#include "google_apis/gaia/gaia_oauth_client.h"
#include "google_apis/gaia/oauth2_token_service.h"
#include "net/url_request/url_request_context_getter.h"

class GoogleServiceAuthError;
class Profile;
class ProfileOAuth2TokenService;

namespace chromeos {

// This class is responsible for restoring authenticated web sessions out of
// OAuth2 refresh tokens or pre-authenticated cookie jar.
class OAuth2LoginManager : public KeyedService,
                           public gaia::GaiaOAuthClient::Delegate,
                           public OAuth2LoginVerifier::Delegate,
                           public OAuth2TokenFetcher::Delegate,
                           public OAuth2TokenService::Observer {
 public:
  // Session restore states.
  enum SessionRestoreState {
    // Session restore is not started.
    SESSION_RESTORE_NOT_STARTED = 0,
    // Session restore is being prepared.
    SESSION_RESTORE_PREPARING = 1,
    // Session restore is in progress. We are currently issuing calls to verify
    // stored OAuth tokens and populate cookie jar with GAIA credentials.
    SESSION_RESTORE_IN_PROGRESS = 2,
    // Session restore is completed.
    SESSION_RESTORE_DONE = 3,
    // Session restore failed.
    SESSION_RESTORE_FAILED = 4,
    // Session restore failed due to connection or service errors.
    SESSION_RESTORE_CONNECTION_FAILED = 5,
  };

  // Session restore strategy.
  enum SessionRestoreStrategy {
    // Generate OAuth2 refresh token from authentication profile's cookie jar.
    // Restore session from generated OAuth2 refresh token.
    RESTORE_FROM_COOKIE_JAR,
    // Restore session from saved OAuth2 refresh token from TokenServices.
    RESTORE_FROM_SAVED_OAUTH2_REFRESH_TOKEN,
    // Restore session from OAuth2 refresh token passed via command line.
    RESTORE_FROM_PASSED_OAUTH2_REFRESH_TOKEN,
  };

  class Observer {
   public:
    virtual ~Observer() {}

    // Raised when merge session state changes.
    virtual void OnSessionRestoreStateChanged(Profile* user_profile,
                                              SessionRestoreState state) {}

    // Raised when a new OAuth2 refresh token is avaialble.
    virtual void OnNewRefreshTokenAvaiable(Profile* user_profile) {}

    // Raised when session's GAIA credentials (SID+LSID) are available to
    // other signed in services.
    virtual void OnSessionAuthenticated(Profile* user_profile) {}
  };

  explicit OAuth2LoginManager(Profile* user_profile);
  ~OAuth2LoginManager() override;

  void AddObserver(OAuth2LoginManager::Observer* observer);
  void RemoveObserver(OAuth2LoginManager::Observer* observer);

  // Restores and verifies OAuth tokens following specified |restore_strategy|.
  // For |restore_strategy| RESTORE_FROM_PASSED_OAUTH2_REFRESH_TOKEN, parameter
  // |oauth2_refresh_token| needs to have a non-empty value.
  // For |restore_strategy| RESTORE_FROM_COOKIE_JAR |auth_request_context| must
  // be initialized.
  void RestoreSession(net::URLRequestContextGetter* auth_request_context,
                      SessionRestoreStrategy restore_strategy,
                      const std::string& oauth2_refresh_token,
                      const std::string& oauth2_access_token);

  // Continues session restore after transient network errors.
  void ContinueSessionRestore();

  // Start restoring session from saved OAuth2 refresh token.
  void RestoreSessionFromSavedTokens();

  // Stops all background authentication requests.
  void Stop();

  // Returns session restore state.
  SessionRestoreState state() { return state_; }

  const base::Time& session_restore_start() { return session_restore_start_; }

  // Returns true if the tab loading should block until session restore
  // finishes.
  bool ShouldBlockTabLoading();

 private:
  friend class MergeSessionLoadPageTest;
  friend class OAuth2Test;

  // Session restore outcomes (for UMA).
  enum SessionRestoreOutcome {
    SESSION_RESTORE_UNDEFINED = 0,
    SESSION_RESTORE_SUCCESS = 1,
    SESSION_RESTORE_TOKEN_FETCH_FAILED = 2,
    SESSION_RESTORE_NO_REFRESH_TOKEN_FAILED = 3,
    SESSION_RESTORE_OAUTHLOGIN_FAILED = 4,
    SESSION_RESTORE_MERGE_SESSION_FAILED = 5,
    SESSION_RESTORE_LISTACCOUNTS_FAILED = 6,
    SESSION_RESTORE_NOT_NEEDED = 7,
    SESSION_RESTORE_COUNT = 8,
  };

  // Outcomes of post-merge session verification.
  // This enum is used for an UMA histogram, and hence new items should only be
  // appended at the end.
  enum MergeVerificationOutcome {
    POST_MERGE_UNDEFINED  = 0,
    POST_MERGE_SUCCESS = 1,
    POST_MERGE_NO_ACCOUNTS = 2,
    POST_MERGE_MISSING_PRIMARY_ACCOUNT = 3,
    POST_MERGE_PRIMARY_NOT_FIRST_ACCOUNT = 4,
    POST_MERGE_VERIFICATION_FAILED = 5,
    POST_MERGE_CONNECTION_FAILED = 6,
    POST_MERGE_COUNT = 7,
  };

  // KeyedService implementation.
  void Shutdown() override;

  // gaia::GaiaOAuthClient::Delegate overrides.
  void OnRefreshTokenResponse(const std::string& access_token,
                              int expires_in_seconds) override;
  void OnGetUserInfoResponse(
      scoped_ptr<base::DictionaryValue> user_info) override;
  void OnOAuthError() override;
  void OnNetworkError(int response_code) override;

  // OAuth2LoginVerifier::Delegate overrides.
  void OnSessionMergeSuccess() override;
  void OnSessionMergeFailure(bool connection_error) override;
  void OnListAccountsSuccess(const std::string& data) override;
  void OnListAccountsFailure(bool connection_error) override;

  // OAuth2TokenFetcher::Delegate overrides.
  void OnOAuth2TokensAvailable(
      const GaiaAuthConsumer::ClientOAuthResult& oauth2_tokens) override;
  void OnOAuth2TokensFetchFailed() override;

  // OAuth2TokenService::Observer implementation:
  void OnRefreshTokenAvailable(const std::string& account_id) override;

  // Signals delegate that authentication is completed, kicks off token fetching
  // process.
  void CompleteAuthentication();

  // Retrieves ProfileOAuth2TokenService for |user_profile_|.
  ProfileOAuth2TokenService* GetTokenService();

  // Retrieves the primary account for |user_profile_|.
  const std::string& GetPrimaryAccountId();

  // Records |refresh_token_| to token service. The associated account id is
  // assumed to be the primary account id of the user profile. If the primary
  // account id is not present, GetAccountInfoOfRefreshToken will be called to
  // retrieve the associated account info.
  void StoreOAuth2Token();

  // Get the account info corresponding to the specified refresh token.
  void GetAccountInfoOfRefreshToken(const std::string& refresh_token);

  // Update the token service and inform listeners of a new refresh token.
  void UpdateCredentials(const std::string& account_id);

  // Attempts to fetch OAuth2 tokens by using pre-authenticated cookie jar from
  // provided |auth_profile|.
  void FetchOAuth2Tokens();

  // Reports when all tokens are loaded.
  void ReportOAuth2TokensLoaded();

  // Checks if primary account sessions cookies are stale and restores them
  // if needed.
  void VerifySessionCookies();

  // Issue GAIA cookie recovery (MergeSession) from |refresh_token_|.
  void RestoreSessionCookies();

  // Checks GAIA error and figures out whether the request should be
  // re-attempted.
  bool RetryOnError(const GoogleServiceAuthError& error);

  // Changes |state_|, if needed fires observers (OnSessionRestoreStateChanged).
  void SetSessionRestoreState(SessionRestoreState state);

  // Testing helper.
  void SetSessionRestoreStartForTesting(const base::Time& time);

  // Records |outcome| of session restore process and sets new |state|.
  void RecordSessionRestoreOutcome(SessionRestoreOutcome outcome,
                                   SessionRestoreState state);

  // Callback from TokenHandleUtil, used to release util.
  void OnTokenHandleComplete(const user_manager::UserID& user_id,
                             TokenHandleUtil::TokenHandleStatus status);

  // Records |outcome| of merge verification check. |is_pre_merge| specifies
  // if this is pre or post merge session verification.
  static void RecordCookiesCheckOutcome(
      bool is_pre_merge,
      MergeVerificationOutcome outcome);

  // Keeps the track if we have already reported OAuth2 token being loaded
  // by OAuth2TokenService.
  Profile* user_profile_;
  scoped_refptr<net::URLRequestContextGetter> auth_request_context_;
  SessionRestoreStrategy restore_strategy_;
  SessionRestoreState state_;

  scoped_ptr<OAuth2TokenFetcher> oauth2_token_fetcher_;
  scoped_ptr<OAuth2LoginVerifier> login_verifier_;
  scoped_ptr<gaia::GaiaOAuthClient> account_info_fetcher_;
  scoped_ptr<TokenHandleUtil> token_handle_util_;

  // OAuth2 refresh token.
  std::string refresh_token_;

  // OAuthLogin scoped access token.
  std::string oauthlogin_access_token_;

  // Session restore start time.
  base::Time session_restore_start_;

  // List of observers to notify when token availability changes.
  // Makes sure list is empty on destruction.
  // TODO(zelidrag|gspencer): Figure out how to get rid of ProfileHelper so we
  // can change the line below to ObserverList<Observer, true>.
  ObserverList<Observer, false> observer_list_;

  base::WeakPtrFactory<OAuth2LoginManager> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(OAuth2LoginManager);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_LOGIN_SIGNIN_OAUTH2_LOGIN_MANAGER_H_
