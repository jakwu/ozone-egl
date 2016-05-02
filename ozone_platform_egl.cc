// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone_platform_egl.h"
#include "egl_surface_factory.h"
#include "egl_window.h"

#include "ui/ozone/public/cursor_factory_ozone.h"
#include "ui/ozone/public/gpu_platform_support.h"
#include "ui/ozone/public/gpu_platform_support_host.h"
#include "ui/events/ozone/device/device_manager.h"
#include "ui/events/ozone/evdev/event_factory_evdev.h"
#include "ui/ozone/public/ozone_platform.h"
#include "egl_wrapper.h"

#include "ui/ozone/common/native_display_delegate_ozone.h"

namespace ui {

namespace {

class OzonePlatformEgl : public OzonePlatform {
 public:
  OzonePlatformEgl()
  {
  }
  ~OzonePlatformEgl() override {
  }

  // OzonePlatform:
  ui::SurfaceFactoryOzone* GetSurfaceFactoryOzone() override {
    return surface_factory_ozone_.get();
  }
  CursorFactoryOzone* GetCursorFactoryOzone() override {
    return cursor_factory_ozone_.get();
  }
  GpuPlatformSupport* GetGpuPlatformSupport() override {
    return gpu_platform_support_.get();
  }
  GpuPlatformSupportHost* GetGpuPlatformSupportHost() override {
    return gpu_platform_support_host_.get();
  }

  scoped_ptr<PlatformWindow> CreatePlatformWindow(
    PlatformWindowDelegate* delegate,
    const gfx::Rect& bounds) override {
    return  make_scoped_ptr<PlatformWindow>(
        new eglWindow(delegate,
                      surface_factory_ozone_.get(),
                      event_factory_ozone_.get(),
                      bounds));
  }
  scoped_ptr<NativeDisplayDelegate> CreateNativeDisplayDelegate() override {
    return scoped_ptr<NativeDisplayDelegate>(new NativeDisplayDelegateOzone());
  }

  void InitializeUI() override {
    device_manager_ = CreateDeviceManager();
    if(!surface_factory_ozone_)
      surface_factory_ozone_.reset(new SurfaceFactoryEgl());
    event_factory_ozone_.reset(
        new EventFactoryEvdev(NULL, device_manager_.get()));
    cursor_factory_ozone_.reset(new CursorFactoryOzone());
    gpu_platform_support_host_.reset(CreateStubGpuPlatformSupportHost());
  }

  void InitializeGPU() override {
    if(!surface_factory_ozone_)
      surface_factory_ozone_.reset(new SurfaceFactoryEgl());
    gpu_platform_support_.reset(CreateStubGpuPlatformSupport());
 }

 private:
  scoped_ptr<DeviceManager> device_manager_;
  scoped_ptr<SurfaceFactoryEgl> surface_factory_ozone_;
  scoped_ptr<EventFactoryEvdev> event_factory_ozone_;
  scoped_ptr<CursorFactoryOzone> cursor_factory_ozone_;

  scoped_ptr<GpuPlatformSupport> gpu_platform_support_;
  scoped_ptr<GpuPlatformSupportHost> gpu_platform_support_host_;

  DISALLOW_COPY_AND_ASSIGN(OzonePlatformEgl);
};

}  // namespace

OzonePlatform* CreateOzonePlatformEgl() {
  OzonePlatformEgl* platform = new OzonePlatformEgl;
  return platform;
}
}  // namespace ui
