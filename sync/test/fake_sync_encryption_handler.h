// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYNC_SYNCABLE_TEST_FAKE_SYNC_ENCRYPTION_HANDLER_H_
#define SYNC_SYNCABLE_TEST_FAKE_SYNC_ENCRYPTION_HANDLER_H_

#include <string>

#include "base/compiler_specific.h"
#include "base/observer_list.h"
#include "sync/internal_api/public/sync_encryption_handler.h"
#include "sync/syncable/nigori_handler.h"
#include "sync/test/fake_encryptor.h"
#include "sync/util/cryptographer.h"

namespace syncer {

// A fake sync encryption handler capable of keeping track of the encryption
// state without opening any transactions or interacting with the nigori node.
// Note that this only performs basic interactions with the cryptographer
// (setting pending keys, installing keys).
// Note: NOT thread safe. If threads attempt to check encryption state
// while another thread is modifying it, races can occur.
class FakeSyncEncryptionHandler : public SyncEncryptionHandler,
                                  public syncable::NigoriHandler {
 public:
  FakeSyncEncryptionHandler();
  ~FakeSyncEncryptionHandler() override;

  // SyncEncryptionHandler implementation.
  void AddObserver(Observer* observer) override;
  void RemoveObserver(Observer* observer) override;
  void Init() override;
  void SetEncryptionPassphrase(const std::string& passphrase,
                               bool is_explicit) override;
  void SetDecryptionPassphrase(const std::string& passphrase) override;
  void EnableEncryptEverything() override;
  bool IsEncryptEverythingEnabled() const override;
  PassphraseType GetPassphraseType() const override;

  // NigoriHandler implemenation.
  void ApplyNigoriUpdate(const sync_pb::NigoriSpecifics& nigori,
                         syncable::BaseTransaction* const trans) override;
  void UpdateNigoriFromEncryptedTypes(
      sync_pb::NigoriSpecifics* nigori,
      syncable::BaseTransaction* const trans) const override;
  bool NeedKeystoreKey(syncable::BaseTransaction* const trans) const override;
  bool SetKeystoreKeys(
      const google::protobuf::RepeatedPtrField<google::protobuf::string>& keys,
      syncable::BaseTransaction* const trans) override;
  ModelTypeSet GetEncryptedTypes(
      syncable::BaseTransaction* const trans) const override;

  Cryptographer* cryptographer() { return &cryptographer_; }

 private:
  base::ObserverList<SyncEncryptionHandler::Observer> observers_;
  ModelTypeSet encrypted_types_;
  bool encrypt_everything_;
  PassphraseType passphrase_type_;

  FakeEncryptor encryptor_;
  Cryptographer cryptographer_;
  std::string keystore_key_;
};

}  // namespace syncer

#endif  // SYNC_INTERNAL_API_PUBLIC_TEST_FAKE_SYNC_ENCRYPTION_HANDLER_H_
