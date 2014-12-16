// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_BROWSER_SERVICE_CAST_SERVICE_H_
#define CHROMECAST_BROWSER_SERVICE_CAST_SERVICE_H_

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/scoped_ptr.h"

class PrefService;

namespace base {
class ThreadChecker;
}

namespace content {
class BrowserContext;
}

namespace net {
class URLRequestContextGetter;
}

namespace chromecast {

namespace metrics {
class CastMetricsServiceClient;
}

class CastService {
 public:
  // Create() takes a separate url request context getter because the request
  // context getter obtained through the browser context might not be
  // appropriate for the url requests made by the cast service/reciever.
  // For example, on Chromecast, it is needed to pass in a system url request
  // context getter that would set the request context for NSS, which the main
  // getter doesn't do.
  static scoped_ptr<CastService> Create(
      content::BrowserContext* browser_context,
      PrefService* pref_service,
      metrics::CastMetricsServiceClient* metrics_service_client,
      net::URLRequestContextGetter* request_context_getter);

  virtual ~CastService();

  // Starts/stops the cast service.
  void Start();
  void Stop();

 protected:
  CastService(content::BrowserContext* browser_context,
              PrefService* pref_service,
              metrics::CastMetricsServiceClient* metrics_service_client);

  // Implementation-specific initialization. Initialization of cast service's
  // sub-components should go here. Anything that should happen before cast
  // service is started but doesn't need the sub-components to finish
  // initializing should also go here.
  virtual void InitializeInternal() = 0;

  // Implementation-specific finalization. Any initializations done by
  // InitializeInternal() should be finalized here.
  virtual void FinalizeInternal() = 0;

  // Implementation-specific start behavior. It basically starts the
  // sub-component services and does additional initialization that cannot be
  // done in the InitializationInternal().
  virtual void StartInternal() = 0;

  // Implementation-specific stop behavior. Any initializations done by
  // StartInternal() should be finalized here.
  virtual void StopInternal() = 0;

  content::BrowserContext* browser_context() const { return browser_context_; }
  PrefService* pref_service() const { return pref_service_; }
  metrics::CastMetricsServiceClient* metrics_service_client() const {
    return metrics_service_client_;
  }

 private:
  content::BrowserContext* const browser_context_;
  PrefService* const pref_service_;
  metrics::CastMetricsServiceClient* const metrics_service_client_;
  bool stopped_;
  const scoped_ptr<base::ThreadChecker> thread_checker_;

  DISALLOW_COPY_AND_ASSIGN(CastService);
};

}  // namespace chromecast

#endif  // CHROMECAST_BROWSER_SERVICE_CAST_SERVICE_H_
