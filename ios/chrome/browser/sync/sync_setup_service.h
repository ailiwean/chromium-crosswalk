// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_SYNC_SYNC_SETUP_SERVICE_H_
#define IOS_CHROME_BROWSER_SYNC_SYNC_SETUP_SERVICE_H_

#include <map>

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "components/keyed_service/core/keyed_service.h"
#include "sync/internal_api/public/base/model_type.h"
#include "sync/internal_api/public/util/syncer_error.h"

namespace sync_driver {
class SyncService;
}

class PrefService;

// Class that allows configuring sync. It handles enabling and disabling it, as
// well as choosing datatypes. Most actions are delayed until a commit is done,
// to allow the complex sync setup flow on iOS.
class SyncSetupService : public KeyedService {
 public:
  typedef enum {
    kNoSyncServiceError,
    kSyncServiceSignInNeedsUpdate,
    kSyncServiceCouldNotConnect,
    kSyncServiceServiceUnavailable,
    kSyncServiceNeedsPassphrase,
    kSyncServiceUnrecoverableError,
    kLastSyncServiceError = kSyncServiceUnrecoverableError
  } SyncServiceState;

  // The set of user-selectable datatypes handled by Chrome for iOS.
  typedef enum {
    kSyncBookmarks,
    kSyncOmniboxHistory,
    kSyncPasswords,
    kSyncOpenTabs,
    kSyncAutofill,
    kNumberOfSyncableDatatypes
  } SyncableDatatype;

  SyncSetupService(sync_driver::SyncService* sync_service, PrefService* prefs);
  ~SyncSetupService() override;

  // Returns the |syncer::ModelType| associated to the given
  // |SyncableDatatypes|.
  syncer::ModelType GetModelType(SyncableDatatype datatype);

  // Returns whether sync is enabled.
  virtual bool IsSyncEnabled() const;
  // Enables or disables sync. Changes won't take effect in the sync backend
  // before the next call to |CommitChanges|.
  virtual void SetSyncEnabled(bool sync_enabled);

  // Returns all currently enabled datatypes.
  syncer::ModelTypeSet GetDataTypes() const;
  // Returns whether the given datatype is enabled.
  virtual bool IsDataTypeEnabled(syncer::ModelType datatype) const;
  // Enables or disables the given datatype. To be noted: this can be called at
  // any time, but will only be meaningful if |IsSyncEnabled| is true and
  // |IsSyncingAllDataTypes| is false. Changes won't take effect in the sync
  // backend before the next call to |CommitChanges|.
  void SetDataTypeEnabled(syncer::ModelType datatype, bool enabled);

  // Returns whether the user needs to enter a passphrase or enable sync to make
  // sync work.
  bool UserActionIsRequiredToHaveSyncWork();

  // Returns whether all datatypes are being synced.
  virtual bool IsSyncingAllDataTypes() const;
  // Sets whether all datatypes should be synced or not. Changes won't take
  // effect before the next call to |CommitChanges|.
  virtual void SetSyncingAllDataTypes(bool sync_all);

  // Returns the current sync service state.
  virtual SyncServiceState GetSyncServiceState();

  // Returns true if the user has gone through the initial sync configuration.
  // This method is guaranteed not to start the sync backend so it can be
  // called at start-up.
  bool HasFinishedInitialSetup();

  // Pauses sync allowing the user to configure what data to sync before
  // actually starting to sync data with the server.
  void PrepareForFirstSyncSetup();

  // Commit the current state of the configuration to the sync backend.
  void CommitChanges();

  // Returns true if there are uncommitted sync changes;
  bool HasUncommittedChanges();

 private:
  // Enables or disables sync. Changes won't take effect in the sync backend
  // before the next call to |CommitChanges|. No changes are made to the
  // currently selected datatypes.
  void SetSyncEnabledWithoutChangingDatatypes(bool sync_enabled);

  sync_driver::SyncService* const sync_service_;
  PrefService* const prefs_;
  syncer::ModelTypeSet user_selectable_types_;

  DISALLOW_COPY_AND_ASSIGN(SyncSetupService);
};

#endif  // IOS_CHROME_BROWSER_SYNC_SYNC_SETUP_SERVICE_H_
