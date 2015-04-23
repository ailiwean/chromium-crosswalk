// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ENGAGEMENT_SITE_ENGAGEMENT_SERVICE_FACTORY_H_
#define CHROME_BROWSER_ENGAGEMENT_SITE_ENGAGEMENT_SERVICE_FACTORY_H_

#include "base/memory/singleton.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

class Profile;
class SiteEngagementService;

// Singleton that owns all SiteEngagementServices and associates them with
// Profiles. Listens for the Profile's destruction notification and cleans up
// the associated SiteEngagementService.
//
// The default factory behavior is suitable for this factory as:
// * there should be no site engagement tracking in incognito
// * the site engagement service should be created lazily
// * the site engagement service is needed in tests.
class SiteEngagementServiceFactory : public BrowserContextKeyedServiceFactory {
 public:
  static SiteEngagementService* GetForProfile(Profile* profile);

  static SiteEngagementServiceFactory* GetInstance();

 private:
  friend struct DefaultSingletonTraits<SiteEngagementServiceFactory>;

  SiteEngagementServiceFactory();
  ~SiteEngagementServiceFactory() override;

  // KeyedServiceBaseFactory:
  bool ServiceIsNULLWhileTesting() const override;

  // BrowserContextKeyedServiceFactory:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* profile) const override;

  DISALLOW_COPY_AND_ASSIGN(SiteEngagementServiceFactory);
};

#endif  // CHROME_BROWSER_ENGAGEMENT_SITE_ENGAGEMENT_SERVICE_FACTORY_H_
