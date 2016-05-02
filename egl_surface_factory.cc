// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "egl_surface_factory.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "ui/ozone/public/surface_ozone_egl.h"
#include "ui/ozone/public/surface_ozone_canvas.h"
#include "ui/ozone/public/surface_factory_ozone.h"
#include "ui/gfx/skia_util.h"
#include "ui/gfx/vsync_provider.h"
#include "base/logging.h"
#include "base/files/file_path.h"

#include "egl_wrapper.h"

#ifndef GL_BGRA_EXT
 #define GL_BGRA_EXT 0x80E1
#endif

#include <fcntl.h>    /* For O_RDWR */
#include <unistd.h>   /* For open(), creat() */
#include <linux/fb.h>
#include <sys/ioctl.h>

#define OZONE_EGL_WINDOW_WIDTH 1024
#define OZONE_EGL_WINDOW_HEIGTH 768

const char kDefaultEglSoname[] = "libEGL.so.1";
const char kDefaultGlesSoname[] = "libGLESv2.so.2";

namespace ui {

class EglOzoneCanvas: public ui::SurfaceOzoneCanvas {
 public:
  EglOzoneCanvas();
  ~EglOzoneCanvas() override  ;
  // SurfaceOzoneCanvas overrides:
  virtual skia::RefPtr<SkCanvas> GetCanvas() override {
    return skia::SharePtr<SkCanvas>(surface_->getCanvas());
  }
  void ResizeCanvas(const gfx::Size& viewport_size) override;
  void PresentCanvas(const gfx::Rect& damage) override;
  
  scoped_ptr<gfx::VSyncProvider> CreateVSyncProvider() override {
    return scoped_ptr<gfx::VSyncProvider>();
  }

 private: 
  skia::RefPtr<SkSurface> surface_;
  ozone_egl_UserData userDate_;
};

EglOzoneCanvas::EglOzoneCanvas()
{
    memset(&userDate_,0,sizeof(userDate_));
}
EglOzoneCanvas::~EglOzoneCanvas()
{
    ozone_egl_textureShutDown (&userDate_);
}

void EglOzoneCanvas::ResizeCanvas(const gfx::Size& viewport_size)
{  
  if(userDate_.width == viewport_size.width() && userDate_.height==viewport_size.height())
  {
      return;
  }
  else if(userDate_.width != 0 && userDate_.height !=0)
  {
      ozone_egl_textureShutDown (&userDate_);
  }
  surface_ = skia::AdoptRef(SkSurface::NewRaster(
        SkImageInfo::Make(viewport_size.width(),
                                   viewport_size.height(),
                                   kN32_SkColorType,
                                   kPremul_SkAlphaType)));
  userDate_.width = viewport_size.width();
  userDate_.height = viewport_size.height();
  userDate_.colorType = GL_BGRA_EXT;
  ozone_egl_textureInit ( &userDate_);
}

void EglOzoneCanvas::PresentCanvas(const gfx::Rect& damage)
{ 
    SkImageInfo info;
    size_t row_bytes;
    userDate_.data = (char *) surface_->peekPixels(&info, &row_bytes);
    ozone_egl_textureDraw(&userDate_);
    ozone_egl_swap();
}


class OzoneEgl : public ui::SurfaceOzoneEGL {
 public:
  OzoneEgl(gfx::AcceleratedWidget window_id){
     native_window_ = window_id;
  }
  ~OzoneEgl() override {
     native_window_=0;
  }

  intptr_t GetNativeWindow() override 
  { 
    return native_window_; 
  }

  bool OnSwapBuffers() override
  {
    return true;
  }

  bool ResizeNativeWindow(const gfx::Size& viewport_size) override {
    return true;
  }


  scoped_ptr<gfx::VSyncProvider> CreateVSyncProvider() override {
    return scoped_ptr<gfx::VSyncProvider>();
  }

 private:
  intptr_t native_window_;
};



SurfaceFactoryEgl::SurfaceFactoryEgl():init_(false)
{

}

SurfaceFactoryEgl::~SurfaceFactoryEgl()
{ 
    DestroySingleWindow(); 
}
  
EGLint g_width;
EGLint g_height;
bool SurfaceFactoryEgl::CreateSingleWindow()
{
  struct fb_var_screeninfo fb_var;

  int fb_fd =  open("/dev/fb0", O_RDWR);

  if(init_)
  {
     return true;
  }

  if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &fb_var)) {
    LOG(FATAL) << "failed to get fb var info errno: " << errno;
    g_width = 640;
    g_height = 480;
  } else {
    g_width = fb_var.xres;
    g_height = fb_var.yres;
  }

  close(fb_fd);

  if(!ozone_egl_setup(0, 0, g_width, g_height))
  {
      LOG(FATAL) << "CreateSingleWindow";
      return false;
  }
  init_ = true;
  return true;
}

void SurfaceFactoryEgl::DestroySingleWindow() {
  ozone_egl_destroy();
  init_ = false;
}

intptr_t SurfaceFactoryEgl::GetNativeDisplay() {
  return (intptr_t)ozone_egl_getNativedisp();
}

intptr_t SurfaceFactoryEgl::GetNativeWindow(){
  return (intptr_t)ozone_egl_GetNativeWin();
}

scoped_ptr<ui::SurfaceOzoneEGL>
SurfaceFactoryEgl::CreateEGLSurfaceForWidget(
    gfx::AcceleratedWidget widget) {
  return make_scoped_ptr<ui::SurfaceOzoneEGL>(
      new OzoneEgl(widget));
}

bool SurfaceFactoryEgl::LoadEGLGLES2Bindings(
    AddGLLibraryCallback add_gl_library,
    SetGLGetProcAddressProcCallback set_gl_get_proc_address) { 
  const char* egl_soname = kDefaultEglSoname;
  const char* gles_soname = kDefaultGlesSoname;

  base::NativeLibraryLoadError error;
  base::NativeLibrary egl_library =
      base::LoadNativeLibrary(base::FilePath(egl_soname), &error);
  if (!egl_library) {
    LOG(WARNING) << "Failed to load EGL library: " << error.ToString();
    return false;
  }

  base::NativeLibrary gles_library =
      base::LoadNativeLibrary(base::FilePath(gles_soname), &error);
  if (!gles_library) {
    LOG(WARNING) << "Failed to load GLES library: " << error.ToString();
    base::UnloadNativeLibrary(egl_library);
    return false;
  }

  GLGetProcAddressProc get_proc_address =
      reinterpret_cast<GLGetProcAddressProc>(
          base::GetFunctionPointerFromNativeLibrary(egl_library,
                                                    "eglGetProcAddress"));
  if (!get_proc_address) {
    LOG(ERROR) << "eglGetProcAddress not found.";
    base::UnloadNativeLibrary(egl_library);
    base::UnloadNativeLibrary(gles_library);
    return false;
  }

  set_gl_get_proc_address.Run(get_proc_address);
  add_gl_library.Run(egl_library);
  add_gl_library.Run(gles_library);
  return true;
}

const int32* SurfaceFactoryEgl::GetEGLSurfaceProperties(
    const int32* desired_list) {
  return ozone_egl_getConfigAttribs();
}

scoped_ptr<ui::SurfaceOzoneCanvas> SurfaceFactoryEgl::CreateCanvasForWidget(
      gfx::AcceleratedWidget widget){
  return make_scoped_ptr<SurfaceOzoneCanvas>(new EglOzoneCanvas());
}

}  // namespace ui
