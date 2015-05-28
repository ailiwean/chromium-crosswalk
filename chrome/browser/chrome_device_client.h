// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROME_DEVICE_CLIENT_H_
#define CHROME_BROWSER_CHROME_DEVICE_CLIENT_H_

#include "device/core/device_client.h"

#include "base/compiler_specific.h"
#include "base/macros.h"

// Implementation of device::DeviceClient that returns //device service
// singletons appropriate for use within the Chrome application.
class ChromeDeviceClient : device::DeviceClient {
 public:
  ChromeDeviceClient();
  ~ChromeDeviceClient() override;

  // device::DeviceClient implementation
  device::UsbService* GetUsbService() override;
  void ConnectToUSBDeviceManager(
      mojo::InterfaceRequest<device::usb::DeviceManager> request) override;
  device::HidService* GetHidService() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(ChromeDeviceClient);
};

#endif  // CHROME_BROWSER_CHROME_DEVICE_CLIENT_H_
