// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_TEST_TEST_EXTENSIONS_CLIENT_H_
#define EXTENSIONS_TEST_TEST_EXTENSIONS_CLIENT_H_

#include "base/macros.h"
#include "extensions/common/extensions_client.h"

namespace extensions {

class TestExtensionsClient : public ExtensionsClient {
 public:
  TestExtensionsClient();
  ~TestExtensionsClient() override;

 private:
  void Initialize() override;
  const PermissionMessageProvider& GetPermissionMessageProvider()
      const override;
  const std::string GetProductName() override;
  scoped_ptr<FeatureProvider> CreateFeatureProvider(
      const std::string& name) const override;
  scoped_ptr<JSONFeatureProviderSource> CreateFeatureProviderSource(
      const std::string& name) const override;
  void FilterHostPermissions(
      const URLPatternSet& hosts,
      URLPatternSet* new_hosts,
      std::set<PermissionMessage>* messages) const override;
  void FilterHostPermissions(const URLPatternSet& hosts,
                             URLPatternSet* new_hosts,
                             PermissionIDSet* permissions) const override;
  void SetScriptingWhitelist(const ScriptingWhitelist& whitelist) override;
  const ScriptingWhitelist& GetScriptingWhitelist() const override;
  URLPatternSet GetPermittedChromeSchemeHosts(
      const Extension* extension,
      const APIPermissionSet& api_permissions) const override;
  bool IsScriptableURL(const GURL& url, std::string* error) const override;
  bool IsAPISchemaGenerated(const std::string& name) const override;
  base::StringPiece GetAPISchema(const std::string& name) const override;
  void RegisterAPISchemaResources(ExtensionAPI* api) const override;
  bool ShouldSuppressFatalErrors() const override;
  std::string GetWebstoreBaseURL() const override;
  std::string GetWebstoreUpdateURL() const override;
  bool IsBlacklistUpdateURL(const GURL& url) const override;
  std::set<base::FilePath> GetBrowserImagePaths(
      const Extension* extension) override;

  // A whitelist of extensions that can script anywhere. Do not add to this
  // list (except in tests) without consulting the Extensions team first.
  // Note: Component extensions have this right implicitly and do not need to be
  // added to this list.
  ScriptingWhitelist scripting_whitelist_;

  DISALLOW_COPY_AND_ASSIGN(TestExtensionsClient);
};

}  // namespace extensions

#endif  // EXTENSIONS_TEST_TEST_EXTENSIONS_CLIENT_H_
