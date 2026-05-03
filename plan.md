# CppWindow Working Plan

This file tracks active issues and cleanup tasks while the API is still moving.
Delete items as they are completed and remove this file once the list is empty.

## Current Issues

- [ ] Investigate macOS title-bar tint/color behavior when switching
  `BorderlessFullscreen -> Fullscreen` in `examples/fullscreen_toggle`.
  Current behavior may be Cocoa title-bar translucency or GLFW style-mask timing.
- [ ] Investigate macOS `ExclusiveFullscreen` DPI/content-scale and cursor-size
  changes with a minimal GLFW reproduction. Current expectation is that this is
  platform/display-mode behavior, but it should be verified before changing API
  semantics.
- [ ] Revisit fullscreen transition behavior after the macOS-specific issues are
  understood. Keep current `Fullscreen` as decorated maximized window for now.
- [ ] Track Ubuntu sanitizer GLX/X11 leak suppression. The current LSan
  suppression targets `extensionSupportedGLX`, which appears to be retained
  Mesa/GLX/X11 memory triggered by GLFW OpenGL context initialization under
  `xvfb`, not cppwindow-owned memory. Revisit if CI images or GLFW behavior
  change.

## Future Work

- [ ] Add optional Vulkan examples behind `CPPWINDOW_BUILD_VULKAN_EXAMPLES`.
  Use `find_package(Vulkan)` and skip with a clear CMake status message when
  Vulkan headers/libs are unavailable. Keep Vulkan out of default builds,
  package tests, and normal CI unless explicitly enabled.
