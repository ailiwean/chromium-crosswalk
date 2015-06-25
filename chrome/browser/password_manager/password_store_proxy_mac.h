// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PASSWORD_MANAGER_PASSWORD_STORE_PROXY_MAC_H_
#define CHROME_BROWSER_PASSWORD_MANAGER_PASSWORD_STORE_PROXY_MAC_H_

#include "base/threading/thread.h"
#include "components/password_manager/core/browser/password_store.h"

namespace crypto {
class AppleKeychain;
}

namespace password_manager {
class LoginDatabase;
}

class PasswordStoreMac;
class SimplePasswordStoreMac;

// The class is a proxy for either PasswordStoreMac or SimplePasswordStoreMac.
// It is responsible for performing migration from PasswordStoreMac to
// SimplePasswordStoreMac and instantiating a correct backend according to the
// user's state.
class PasswordStoreProxyMac : public password_manager::PasswordStore {
 public:
  PasswordStoreProxyMac(
      scoped_refptr<base::SingleThreadTaskRunner> main_thread_runner,
      scoped_ptr<crypto::AppleKeychain> keychain,
      scoped_ptr<password_manager::LoginDatabase> login_db);

  bool Init(const syncer::SyncableService::StartSyncFlare& flare) override;
  void Shutdown() override;
  scoped_refptr<base::SingleThreadTaskRunner> GetBackgroundTaskRunner()
      override;

#if defined(UNIT_TEST)
  password_manager::LoginDatabase* login_metadata_db() {
    return login_metadata_db_.get();
  }
#endif

 private:
  ~PasswordStoreProxyMac() override;

  password_manager::PasswordStore* GetBackend() const;

  // Opens LoginDatabase on the background |thread_|.
  void InitOnBackgroundThread();

  // PasswordStore:
  void ReportMetricsImpl(const std::string& sync_username,
                         bool custom_passphrase_sync_enabled) override;
  password_manager::PasswordStoreChangeList AddLoginImpl(
      const autofill::PasswordForm& form) override;
  password_manager::PasswordStoreChangeList UpdateLoginImpl(
      const autofill::PasswordForm& form) override;
  password_manager::PasswordStoreChangeList RemoveLoginImpl(
      const autofill::PasswordForm& form) override;
  password_manager::PasswordStoreChangeList RemoveLoginsCreatedBetweenImpl(
      base::Time delete_begin,
      base::Time delete_end) override;
  password_manager::PasswordStoreChangeList RemoveLoginsSyncedBetweenImpl(
      base::Time delete_begin,
      base::Time delete_end) override;
  ScopedVector<autofill::PasswordForm> FillMatchingLogins(
      const autofill::PasswordForm& form,
      AuthorizationPromptPolicy prompt_policy) override;
  bool FillAutofillableLogins(
      ScopedVector<autofill::PasswordForm>* forms) override;
  bool FillBlacklistLogins(
      ScopedVector<autofill::PasswordForm>* forms) override;
  void AddSiteStatsImpl(
      const password_manager::InteractionsStats& stats) override;
  void RemoveSiteStatsImpl(const GURL& origin_domain) override;
  scoped_ptr<password_manager::InteractionsStats> GetSiteStatsImpl(
      const GURL& origin_domain) override;

  scoped_refptr<PasswordStoreMac> password_store_mac_;
  scoped_refptr<SimplePasswordStoreMac> password_store_simple_;

  // The login metadata SQL database. If opening the DB on |thread_| fails,
  // |login_metadata_db_| will be reset to NULL for the lifetime of |this|.
  scoped_ptr<password_manager::LoginDatabase> login_metadata_db_;

  // Thread that the synchronous methods are run on.
  scoped_ptr<base::Thread> thread_;

  DISALLOW_COPY_AND_ASSIGN(PasswordStoreProxyMac);
};

#endif  // CHROME_BROWSER_PASSWORD_MANAGER_PASSWORD_STORE_PROXY_MAC_H_
