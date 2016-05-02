#ifndef sghsdjfghfdjkxg
#define sghsdjfghfdjkxg


#include "ui/events/platform/platform_event_dispatcher.h"
#include "ui/platform_window/platform_window.h"
#include "ui/platform_window/platform_window_delegate.h"
#include "ui/events/ozone/evdev/event_factory_evdev.h"
#include "ui/gfx/rect.h"

namespace ui {
class SurfaceFactoryEgl;

class eglWindow : public PlatformWindow, public PlatformEventDispatcher {
 public:
  eglWindow(PlatformWindowDelegate* delegate,
          SurfaceFactoryEgl* surface_factory,
          EventFactoryEvdev* event_factory,
          const gfx::Rect& bounds);
  ~eglWindow() override;

  // PlatformWindow:
  gfx::Rect GetBounds() override;
  void SetBounds(const gfx::Rect& bounds) override;
  void Show() override;
  void Hide() override;
  void Close() override;
  void SetCapture() override;
  void ReleaseCapture() override;
  void ToggleFullscreen() override;
  void Maximize() override;
  void Minimize() override;
  void Restore() override;
  void SetCursor(PlatformCursor cursor) override;
  void MoveCursorTo(const gfx::Point& location) override;

  // PlatformEventDispatcher:
  bool CanDispatchEvent(const PlatformEvent& event) override;
  uint32_t DispatchEvent(const PlatformEvent& event) override;

 private:
  PlatformWindowDelegate* delegate_;
  SurfaceFactoryEgl* surface_factory_;
  EventFactoryEvdev* event_factory_;
  gfx::Rect bounds_;
  intptr_t window_id_;


  DISALLOW_COPY_AND_ASSIGN(eglWindow);
};

} // namespace ui

#endif
