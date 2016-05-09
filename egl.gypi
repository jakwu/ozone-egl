# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'internal_ozone_platform_deps': [
      'ozone_platform_egl',
    ],
    'internal_ozone_platforms': [
      'egl'
    ],
  },
  'targets': [
    {
      'target_name': 'ozone_platform_egl',
      'type': 'static_library',
      'defines': [
        'OZONE_IMPLEMENTATION',
        'LINUX',
        'EGL_API_FB'
      ],
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/ui/events/events.gyp:events',
        '<(DEPTH)/ui/events/ozone/events_ozone.gyp:events_ozone_evdev',
        '<(DEPTH)/ui/gfx/gfx.gyp:gfx',
      ],
      'sources': [
        'ozone_platform_egl.cc',
        'ozone_platform_egl.h',
        'egl_surface_factory.cc',
        'egl_surface_factory.h',
        'ozone_platform_egl.h',
        'ozone_platform_egl.cc',
        'egl_wrapper.cc',
        'egl_wrapper.h',
        'egl_window.cc',
        'egl_window.h',
      ],
      'link_settings': {
            'libraries': [
              '-lEGL',
              '-lGLESv2',
            ],
      },
    },
  ],
}
