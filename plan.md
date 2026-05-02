# CppWindow Improvement Plan

Temporary working checklist. Remove this file after the planned work is complete.

## Scope Rules

- Keep the user in control of the main loop.
- Keep CppWindow focused on windowing, input, events, timing helpers, and platform utilities.
- Do not add a renderer to the core library.
- Prefer small, composable APIs over framework-style ownership.

## Phase 1: Better Window Controls

- [x] Add public window state enums and structs:
  - [x] `CursorMode`: `Normal`, `Hidden`, `Captured`
  - [x] `WindowMode`: `Windowed`, `Fullscreen`, `BorderlessFullscreen`, `ExclusiveFullscreen`
  - [x] `MonitorInfo`: name, position, physical size, content scale, current video mode
  - [x] `VideoMode`: width, height, refresh rate, color bit depth
- [x] Add window control APIs:
  - [x] `setPosition(int x, int y)`
  - [x] `getPosition()`
  - [x] `setSizeLimits(...)`
  - [x] `setAspectRatio(...)`
  - [x] `setResizable(bool)`
  - [x] `setDecorated(bool)`
  - [x] `setFloating(bool)`
  - [x] `setOpacity(float)`
  - [x] `getOpacity()`
  - [x] `setVSync(bool)`
  - [x] `setCursorMode(CursorMode)`
  - [x] `getCursorMode()`
  - [x] `minimize()`
  - [x] `maximize()`
  - [x] `restore()`
  - [x] `setWindowMode(WindowMode, optional monitor/video mode)`
  - [x] `getWindowMode()`
- [x] Add monitor APIs on `WindowContext`:
  - [x] `getMonitors()`
  - [x] `getPrimaryMonitor()`
  - [x] `getVideoModes(monitor)`
  - [x] `getContentScale()`
- [x] Add events:
  - [x] `Moved`
  - [x] `Minimized`
  - [x] `Restored`
  - [x] `Maximized`
  - [x] `DpiChanged` or `ContentScaleChanged`
  - [x] `MonitorChanged`
- [x] Update GLFW backend implementation.
- [x] Add tests for non-window pure mapping/state code where possible.
- [x] Add examples:
  - [x] `window_controls.cpp`
  - [x] `fullscreen_toggle.cpp`
  - [x] `mouse_capture.cpp`

## Phase 2: Input Quality

- [ ] Add mouse delta tracking.
- [ ] Add cursor enter/leave state.
- [ ] Add text input example.
- [ ] Add gamepad support:
  - [ ] connection events
  - [ ] buttons
  - [ ] axes
  - [ ] standard gamepad mapping

## Phase 3: App Utility APIs

- [ ] Add clipboard text APIs.
- [ ] Add file drag-and-drop events.
- [ ] Add timer helpers that do not own the loop:
  - [ ] `Clock`
  - [ ] `FrameTimer`
  - [ ] optional fixed-step accumulator helper

## Phase 4: Documentation and Polish

- [x] Expand README with the new window control APIs.
- [x] Add a short "Build an App/Game" guide.
- [ ] Keep examples small and focused.
- [ ] Verify install/export path after API additions.
- [ ] Remove this `plan.md` file after all checklist items are done.
