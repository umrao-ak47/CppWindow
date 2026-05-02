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

## Next Work

- [ ] Testing and CI
  - [ ] Add sanitizer builds where supported.
  - [ ] Ensure CI compiles all examples on macOS, Linux, and Windows.
  - [ ] Add more compile-time API regression tests.
- [ ] Docs and examples
  - [ ] Add recipe-style examples for game loop, editor/tool loop, action
        contexts, multi-window, and fullscreen mode selection.
  - [ ] Document platform-specific fullscreen behavior after investigation.
