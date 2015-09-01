// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PASSWORD_MANAGER_SYNC_BROWSER_SYNC_STORE_RESULT_FILTER_H_
#define COMPONENTS_PASSWORD_MANAGER_SYNC_BROWSER_SYNC_STORE_RESULT_FILTER_H_

#include <string>

#include "base/callback.h"
#include "components/autofill/core/common/password_form.h"
#include "components/password_manager/core/browser/credentials_filter.h"
#include "components/password_manager/core/browser/password_manager_client.h"
#include "components/signin/core/browser/signin_manager.h"
#include "components/sync_driver/sync_service.h"

namespace password_manager {

// The sync- and GAIA- aware implementation of the filter.
// TODO(vabr): Rename this to match the interface.
class SyncStoreResultFilter : public CredentialsFilter {
 public:
  typedef base::Callback<const sync_driver::SyncService*(void)>
      SyncServiceFactoryFunction;
  typedef base::Callback<const SigninManagerBase*(void)>
      SigninManagerFactoryFunction;

  // Implements protection of sync credentials. Uses |client| to get the last
  // commited entry URL for a check against GAIA reauth site. Uses the factory
  // functions repeatedly to get the sync service and signin manager to pass
  // them to sync_util methods.
  // TODO(vabr): Could we safely just get a pointer to the services for the
  // lifetime of the filter?
  SyncStoreResultFilter(
      const PasswordManagerClient* client,
      SyncServiceFactoryFunction sync_service_factory_function,
      SigninManagerFactoryFunction signin_manager_factory_function);
  ~SyncStoreResultFilter() override;

  // CredentialsFilter
  ScopedVector<autofill::PasswordForm> FilterResults(
      ScopedVector<autofill::PasswordForm> results) const override;
  bool ShouldSave(const autofill::PasswordForm& form) const override;
  void ReportFormUsed(const autofill::PasswordForm& form) const override;

 private:
  enum AutofillForSyncCredentialsState {
    ALLOW_SYNC_CREDENTIALS,
    DISALLOW_SYNC_CREDENTIALS_FOR_REAUTH,
    DISALLOW_SYNC_CREDENTIALS,
  };

  // Determines autofill state based on experiment and flag values.
  static AutofillForSyncCredentialsState GetAutofillForSyncCredentialsState();

  const PasswordManagerClient* const client_;

  const SyncServiceFactoryFunction sync_service_factory_function_;

  const SigninManagerFactoryFunction signin_manager_factory_function_;

  DISALLOW_COPY_AND_ASSIGN(SyncStoreResultFilter);
};

}  // namespace password_manager

#endif  // COMPONENTS_PASSWORD_MANAGER_SYNC_BROWSER_SYNC_STORE_RESULT_FILTER_H_
