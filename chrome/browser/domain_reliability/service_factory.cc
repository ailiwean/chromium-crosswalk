// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/domain_reliability/service_factory.h"

#include "base/command_line.h"
#include "base/metrics/field_trial.h"
#include "chrome/browser/metrics/chrome_metrics_service_accessor.h"
#include "chrome/common/chrome_switches.h"
#include "components/domain_reliability/service.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "content/public/browser/browser_context.h"

#if defined(OS_CHROMEOS)
#include "chrome/browser/chromeos/settings/cros_settings.h"
#endif  // defined(OS_CHROMEOS)

namespace domain_reliability {

namespace {

// If Domain Reliability is enabled in the absence of a flag or field trial.
const bool kDefaultEnabled = true;

// The name and value of the field trial to turn Domain Reliability on.
const char kFieldTrialName[] = "DomRel-Enable";
const char kFieldTrialValueEnable[] = "enable";

// Identifies Chrome as the source of Domain Reliability uploads it sends.
const char kUploadReporterString[] = "chrome";

bool IsDomainReliabilityMonitoringEnabled() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(switches::kDisableDomainReliability))
    return false;
  if (command_line->HasSwitch(switches::kEnableDomainReliability))
    return true;

  if (base::FieldTrialList::TrialExists(kFieldTrialName)) {
    std::string value = base::FieldTrialList::FindFullName(kFieldTrialName);
    return value == kFieldTrialValueEnable;
  }

  return kDefaultEnabled;
}

}  // namespace

// static
DomainReliabilityService*
DomainReliabilityServiceFactory::GetForBrowserContext(
    content::BrowserContext* context) {
  return static_cast<DomainReliabilityService*>(
      GetInstance()->GetServiceForBrowserContext(context,
                                                 /* create = */ true));
}

// static
DomainReliabilityServiceFactory*
DomainReliabilityServiceFactory::GetInstance() {
  return base::Singleton<DomainReliabilityServiceFactory>::get();
}

DomainReliabilityServiceFactory::DomainReliabilityServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "DomainReliabilityService",
          BrowserContextDependencyManager::GetInstance()) {}

DomainReliabilityServiceFactory::~DomainReliabilityServiceFactory() {}

KeyedService* DomainReliabilityServiceFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  if (!IsDomainReliabilityMonitoringEnabled())
    return NULL;
  // TODO(ttuttle): Move this check closer to where the data gets sent
  // crbug.com/533486
  if (!ChromeMetricsServiceAccessor::IsMetricsAndCrashReportingEnabled())
    return NULL;

  return DomainReliabilityService::Create(kUploadReporterString);
}

}  // namespace domain_reliability
