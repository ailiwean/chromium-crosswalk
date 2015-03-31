// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_NETWORK_MANAGED_NETWORK_CONFIGURATION_HANDLER_IMPL_H_
#define CHROMEOS_NETWORK_MANAGED_NETWORK_CONFIGURATION_HANDLER_IMPL_H_

#include <map>
#include <set>
#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/linked_ptr.h"
#include "base/memory/weak_ptr.h"
#include "chromeos/chromeos_export.h"
#include "chromeos/network/managed_network_configuration_handler.h"
#include "chromeos/network/network_handler_callbacks.h"
#include "chromeos/network/network_profile_observer.h"
#include "chromeos/network/policy_applicator.h"

namespace base {
class DictionaryValue;
}

namespace chromeos {

class NetworkConfigurationHandler;
struct NetworkProfile;
class NetworkProfileHandler;
class NetworkStateHandler;

class CHROMEOS_EXPORT ManagedNetworkConfigurationHandlerImpl
    : public ManagedNetworkConfigurationHandler,
      public NetworkProfileObserver,
      public PolicyApplicator::ConfigurationHandler {
 public:
  ~ManagedNetworkConfigurationHandlerImpl() override;

  // ManagedNetworkConfigurationHandler overrides
  void AddObserver(NetworkPolicyObserver* observer) override;
  void RemoveObserver(NetworkPolicyObserver* observer) override;

  void GetProperties(
      const std::string& service_path,
      const network_handler::DictionaryResultCallback& callback,
      const network_handler::ErrorCallback& error_callback) override;

  void GetManagedProperties(
      const std::string& userhash,
      const std::string& service_path,
      const network_handler::DictionaryResultCallback& callback,
      const network_handler::ErrorCallback& error_callback) override;

  void SetProperties(
      const std::string& service_path,
      const base::DictionaryValue& user_settings,
      const base::Closure& callback,
      const network_handler::ErrorCallback& error_callback) override;

  void CreateConfiguration(
      const std::string& userhash,
      const base::DictionaryValue& properties,
      const network_handler::StringResultCallback& callback,
      const network_handler::ErrorCallback& error_callback) const override;

  void RemoveConfiguration(
      const std::string& service_path,
      const base::Closure& callback,
      const network_handler::ErrorCallback& error_callback) const override;

  void SetPolicy(onc::ONCSource onc_source,
                 const std::string& userhash,
                 const base::ListValue& network_configs_onc,
                 const base::DictionaryValue& global_network_config) override;

  bool IsAnyPolicyApplicationRunning() const override;

  const base::DictionaryValue* FindPolicyByGUID(
      const std::string userhash,
      const std::string& guid,
      onc::ONCSource* onc_source) const override;

  const GuidToPolicyMap* GetNetworkConfigsFromPolicy(
      const std::string& userhash) const override;

  const base::DictionaryValue* GetGlobalConfigFromPolicy(
      const std::string& userhash) const override;

  const base::DictionaryValue* FindPolicyByGuidAndProfile(
      const std::string& guid,
      const std::string& profile_path) const override;

  // NetworkProfileObserver overrides
  void OnProfileAdded(const NetworkProfile& profile) override;
  void OnProfileRemoved(const NetworkProfile& profile) override;

  // PolicyApplicator::ConfigurationHandler overrides
  void CreateConfigurationFromPolicy(
      const base::DictionaryValue& shill_properties) override;

  void UpdateExistingConfigurationWithPropertiesFromPolicy(
      const base::DictionaryValue& existing_properties,
      const base::DictionaryValue& new_properties) override;

  void OnPoliciesApplied(const NetworkProfile& profile) override;

 private:
  friend class AutoConnectHandlerTest;
  friend class ClientCertResolverTest;
  friend class ManagedNetworkConfigurationHandlerTest;
  friend class NetworkConnectionHandlerTest;
  friend class NetworkHandler;

  struct Policies;
  typedef base::Callback<void(const std::string& service_path,
                              scoped_ptr<base::DictionaryValue> properties)>
      GetDevicePropertiesCallback;
  typedef std::map<std::string, linked_ptr<Policies> > UserToPoliciesMap;
  typedef std::map<std::string, linked_ptr<PolicyApplicator>>
      UserToPolicyApplicatorMap;
  typedef std::map<std::string, std::set<std::string>>
      UserToModifiedPoliciesMap;

  ManagedNetworkConfigurationHandlerImpl();

  // Handlers may be NULL in tests so long as they do not execute any paths
  // that require the handlers.
  void Init(NetworkStateHandler* network_state_handler,
            NetworkProfileHandler* network_profile_handler,
            NetworkConfigurationHandler* network_configuration_handler,
            NetworkDeviceHandler* network_device_handler);

  // Sends the response to the caller of GetManagedProperties.
  void SendManagedProperties(
      const std::string& userhash,
      const network_handler::DictionaryResultCallback& callback,
      const network_handler::ErrorCallback& error_callback,
      const std::string& service_path,
      scoped_ptr<base::DictionaryValue> shill_properties);

  // Sends the response to the caller of GetProperties.
  void SendProperties(
      const network_handler::DictionaryResultCallback& callback,
      const network_handler::ErrorCallback& error_callback,
      const std::string& service_path,
      scoped_ptr<base::DictionaryValue> shill_properties);

  const Policies* GetPoliciesForUser(const std::string& userhash) const;
  const Policies* GetPoliciesForProfile(const NetworkProfile& profile) const;

  void OnPolicyAppliedToNetwork(const std::string& service_path);

  // Helper method to append associated Device properties to |properties|.
  void GetDeviceStateProperties(const std::string& service_path,
                                base::DictionaryValue* properties);

  // Callback for NetworkConfigurationHandler::GetProperties requests from
  // Get{Managed}Properties. This callback fills in properties from
  // DeviceState and may request additional Device properties.
  // Note: Requesting Device properties requires an additional fetch and
  // additional copying of data, so we only do it for Cellular networks which
  // contain a lot of necessary state in the associated Device object.
  void GetPropertiesCallback(
      GetDevicePropertiesCallback send_callback,
      const std::string& service_path,
      const base::DictionaryValue& shill_properties);

  void GetDevicePropertiesSuccess(
      const std::string& service_path,
      scoped_ptr<base::DictionaryValue> network_properties,
      GetDevicePropertiesCallback send_callback,
      const std::string& device_path,
      const base::DictionaryValue& device_properties);
  void GetDevicePropertiesFailure(
      const std::string& service_path,
      scoped_ptr<base::DictionaryValue> network_properties,
      GetDevicePropertiesCallback send_callback,
      const std::string& error_name,
      scoped_ptr<base::DictionaryValue> error_data);

  // Called from SetProperties, calls NCH::SetShillProperties.
  void SetShillProperties(const std::string& service_path,
                          scoped_ptr<base::DictionaryValue> shill_dictionary,
                          const base::Closure& callback,
                          const network_handler::ErrorCallback& error_callback);

  // Applies policies for |userhash|. |modified_policies| must be not null and
  // contain the GUIDs of the network configurations that changed since the last
  // policy application. Returns true if policy application was started and
  // false if it was queued or delayed.
  bool ApplyOrQueuePolicies(const std::string& userhash,
                            std::set<std::string>* modified_policies);

  // If present, the empty string maps to the device policy.
  UserToPoliciesMap policies_by_user_;

  // Local references to the associated handler instances.
  NetworkStateHandler* network_state_handler_;
  NetworkProfileHandler* network_profile_handler_;
  NetworkConfigurationHandler* network_configuration_handler_;
  NetworkDeviceHandler* network_device_handler_;

  // Owns the currently running PolicyApplicators.
  UserToPolicyApplicatorMap policy_applicators_;

  // Per userhash (or empty string for device policy), contains the GUIDs of the
  // policies that were modified.
  // If this map contains a userhash as key, it means that a policy application
  // for this userhash is pending even if no policies were modified and the
  // associated set of GUIDs is empty.
  UserToModifiedPoliciesMap queued_modified_policies_;

  ObserverList<NetworkPolicyObserver> observers_;

  // For Shill client callbacks
  base::WeakPtrFactory<ManagedNetworkConfigurationHandlerImpl>
      weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(ManagedNetworkConfigurationHandlerImpl);
};

}  // namespace chromeos

#endif  // CHROMEOS_NETWORK_MANAGED_NETWORK_CONFIGURATION_HANDLER_IMPL_H_
