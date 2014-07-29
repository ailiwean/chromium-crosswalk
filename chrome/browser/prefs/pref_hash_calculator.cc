// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/prefs/pref_hash_calculator.h"

#include <vector>

#include "base/bind.h"
#include "base/json/json_string_value_serializer.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/threading/thread_restrictions.h"
#include "base/values.h"
#include "chrome/browser/prefs/tracked/pref_hash_calculator_helper.h"
#include "crypto/hmac.h"

namespace {

// Calculates an HMAC of |message| using |key|, encoded as a hexadecimal string.
std::string GetDigestString(const std::string& key,
                            const std::string& message) {
  crypto::HMAC hmac(crypto::HMAC::SHA256);
  std::vector<uint8> digest(hmac.DigestLength());
  if (!hmac.Init(key) || !hmac.Sign(message, &digest[0], digest.size())) {
    NOTREACHED();
    return std::string();
  }
  return base::HexEncode(&digest[0], digest.size());
}

// Verifies that |digest_string| is a valid HMAC of |message| using |key|.
// |digest_string| must be encoded as a hexadecimal string.
bool VerifyDigestString(const std::string& key,
                        const std::string& message,
                        const std::string& digest_string) {
  crypto::HMAC hmac(crypto::HMAC::SHA256);
  std::vector<uint8> digest;
  return base::HexStringToBytes(digest_string, &digest) &&
      hmac.Init(key) &&
      hmac.Verify(message,
                  base::StringPiece(reinterpret_cast<char*>(&digest[0]),
                                    digest.size()));
}

// Renders |value| as a string. |value| may be NULL, in which case the result
// is an empty string. This method can be expensive and its result should be
// re-used rather than recomputed where possible.
std::string ValueAsString(const base::Value* value) {
  // Dictionary values may contain empty lists and sub-dictionaries. Make a
  // deep copy with those removed to make the hash more stable.
  const base::DictionaryValue* dict_value;
  scoped_ptr<base::DictionaryValue> canonical_dict_value;
  if (value && value->GetAsDictionary(&dict_value)) {
    canonical_dict_value.reset(dict_value->DeepCopyWithoutEmptyChildren());
    value = canonical_dict_value.get();
  }

  std::string value_as_string;
  if (value) {
    JSONStringValueSerializer serializer(&value_as_string);
    serializer.Serialize(*value);
  }

  return value_as_string;
}

// Concatenates |device_id|, |path|, and |value_as_string| to give the hash
// input.
std::string GetMessage(const std::string& device_id,
                       const std::string& path,
                       const std::string& value_as_string) {
  std::string message;
  message.reserve(device_id.size() + path.size() + value_as_string.size());
  message.append(device_id);
  message.append(path);
  message.append(value_as_string);
  return message;
}

// Generates a device ID based on the input device ID. The derived device ID has
// no useful properties beyond those of the input device ID except that it is
// consistent with previous implementations.
std::string GenerateDeviceIdLikePrefMetricsServiceDid(
    const std::string& original_device_id) {
  if (original_device_id.empty())
    return std::string();
  return StringToLowerASCII(
      GetDigestString(original_device_id, "PrefMetricsService"));
}

}  // namespace

PrefHashCalculator::PrefHashCalculator(const std::string& seed,
                                       const std::string& device_id)
    : seed_(seed),
      device_id_(GenerateDeviceIdLikePrefMetricsServiceDid(device_id)),
      raw_device_id_(device_id),
      get_legacy_device_id_callback_(base::Bind(&GetLegacyDeviceId)) {}

PrefHashCalculator::PrefHashCalculator(
    const std::string& seed,
    const std::string& device_id,
    const GetLegacyDeviceIdCallback& get_legacy_device_id_callback)
    : seed_(seed),
      device_id_(GenerateDeviceIdLikePrefMetricsServiceDid(device_id)),
      raw_device_id_(device_id),
      get_legacy_device_id_callback_(get_legacy_device_id_callback) {}

PrefHashCalculator::~PrefHashCalculator() {}

std::string PrefHashCalculator::Calculate(const std::string& path,
                                          const base::Value* value) const {
  return GetDigestString(seed_,
                         GetMessage(device_id_, path, ValueAsString(value)));
}

PrefHashCalculator::ValidationResult PrefHashCalculator::Validate(
    const std::string& path,
    const base::Value* value,
    const std::string& digest_string) const {
  const std::string value_as_string(ValueAsString(value));
  if (VerifyDigestString(seed_, GetMessage(device_id_, path, value_as_string),
                         digest_string)) {
    return VALID;
  }
  if (VerifyDigestString(seed_,
                         GetMessage(RetrieveLegacyDeviceId(), path,
                                    value_as_string),
                         digest_string)) {
    return VALID_SECURE_LEGACY;
  }
  return INVALID;
}

std::string PrefHashCalculator::RetrieveLegacyDeviceId() const {
  if (!legacy_device_id_instance_) {
    // Allow IO on this thread to retrieve the legacy device ID. The result of
    // this operation is stored in |legacy_device_id_instance_| and will thus
    // only happen at most once per PrefHashCalculator. This is not ideal, but
    // this value is required synchronously to be able to continue loading prefs
    // for this profile. This profile should then be migrated to a modern device
    // ID and subsequent loads of this profile shouldn't need to run this code
    // ever again.
    // TODO(gab): Remove this when the legacy device ID (M33) becomes
    // irrelevant.
    base::ThreadRestrictions::ScopedAllowIO allow_io;
    legacy_device_id_instance_.reset(
        new std::string(GenerateDeviceIdLikePrefMetricsServiceDid(
            get_legacy_device_id_callback_.Run(raw_device_id_))));
  }
  return *legacy_device_id_instance_;
}
