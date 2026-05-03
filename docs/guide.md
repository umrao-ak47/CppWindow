# CppWindow Guide

CppWindow is a C++20 windowing and input layer built on GLFW. It is intended
to be the platform layer under a game, tool, renderer, or small graphics app.
It owns windows, event collection, per-window input state, monitor queries, and
native surface/context hooks.

CppWindow does not own the application loop and does not provide a renderer.
Your application decides when to poll, update, render, and present.

For copyable application shapes, see the [Recipes](recipes.md). For a
complete symbol-by-symbol reference, see the [API Reference](api.md).

## Requirements

- CMake 3.20 or newer
- A C++20 compiler
- GLFW 3.4, fetched automatically when no `glfw` target already exists

## Add To A Project

The simplest setup is to add CppWindow as a subdirectory:

```cmake
add_subdirectory(external/CppWindow)
target_link_libraries(my_app PRIVATE cppwindow::cppwindow)
```

Then include the public header:

```cpp
#include <cppwindow/cppwindow.hpp>
```

CppWindow currently builds a static library target named
`cppwindow::cppwindow`.

## Build This Repository

Configure and build:

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

By default, CMake `FetchContent` stores dependencies under each build
directory. This repository changes that default to `.deps/glfw`, so CMake
downloads the GLFW source once and reuses it for `dev`, `sanitizers`,
`install`, and `multi`. Each build directory still compiles GLFW with its own
flags.

```bash
cmake --preset sanitizers
cmake --build --preset sanitizers
ctest --preset sanitizers

cmake --preset docs
cmake --build --preset docs

cmake --preset install
cmake --build --preset install

cd tests/package_consumer
cmake --preset installed
cmake --build --preset installed
ctest --preset installed
```

Use the `multi` preset when you want Debug and Release from one build tree:

```bash
cmake --preset multi
cmake --build --preset multi-debug
cmake --build --preset multi-release
ctest --preset multi-debug
ctest --preset multi-release
```

Useful example targets:

```bash
cmake --build build --target example_basic
cmake --build build --target example_opengl
cmake --build build --target example_window_controls
cmake --build build --target example_fullscreen_toggle
cmake --build build --target example_text_input
cmake --build build --target example_gamepad
cmake --build build --target example_app_utilities
cmake --build build --target example_input_helpers
cmake --build build --target example_multi_window
cmake --build build --target example_event_viewer
cmake --build build --target example_monitor_info
cmake --build build --target example_native_handles
cmake --build build --target example_fixed_step_loop
cmake --build build --target example_particles
```

Build the generated API reference:

```bash
cmake --preset docs
cmake --build --preset docs
```

## Minimal Window

Use `WindowBuilder` to create a window. Use `WindowContext::get()` to poll
platform events.

```cpp
#include <cppwindow/cppwindow.hpp>

int main()
{
    auto& ctx = cwin::WindowContext::get();

    auto window = cwin::WindowBuilder{}
                      .title("CppWindow App")
                      .size(1280, 720)
                      .noGraphicsApi()
                      .resizable()
                      .build();

    cwin::EventDispatcher dispatcher;
    dispatcher.on<cwin::Event::Closed>([&] {
        window.requestClose();
    });

    while (!window.shouldClose()) {
        ctx.pollEvents();
        dispatcher.dispatch(window.events());

        // Update and render here.
    }
}
```

`pollEvents()` clears each window event queue, updates previous input state,
then collects new events for the current frame. Read `window.events()` and
`window.getInput()` after polling.

For event-driven tools that can sleep while idle, use `waitEvents()` or
`waitEventsTimeout(seconds)` instead. `postEmptyEvent()` wakes a thread blocked
inside either wait call.

Builder options can cover most initial window state:

```cpp
auto window = cwin::WindowBuilder{}
                  .title("Configured App")
                  .size(1280, 720)
                  .position(120, 80)
                  .noGraphicsApi()
                  .resizable()
                  .decorated()
                  .floating(false)
                  .sizeLimits({ .minWidth = 640, .minHeight = 360 })
                  .aspectRatio({ 16, 9 })
                  .cursorMode(cwin::CursorMode::Normal)
                  .windowMode(cwin::WindowMode::Windowed)
                  .build();
```

Use `.hidden()`, `.visible(false)`, `.focused(false)`, `.borderless()`,
`.opacity(value)`, `.vSync(enabled)`, or `.windowMode(...)` when those states
should be applied before the first frame instead of immediately after `build()`.

## OpenGL Windows

CppWindow can create an OpenGL context. You still need an OpenGL loader such
as GLAD.

```cpp
auto& ctx = cwin::WindowContext::get();

auto window = cwin::WindowBuilder{}
                  .title("OpenGL App")
                  .size(1280, 720)
                  .openGL({ 4, 1, true })
                  .resizable()
                  .build();

window.makeContextCurrent();
gladLoadGLLoader(reinterpret_cast<GLADloadproc>(ctx.getProcLoader()));
window.setVSync(true);

while (!window.shouldClose()) {
    ctx.pollEvents();

    auto [fbWidth, fbHeight] = window.getFramebufferSize();
    glViewport(0, 0, static_cast<GLsizei>(fbWidth), static_cast<GLsizei>(fbHeight));

    glClearColor(0.05f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    window.swapBuffers();
}
```

Use framebuffer size for OpenGL viewport and render target sizing. Window size
is in screen coordinates; framebuffer size is in pixels. They can differ on
high-DPI displays.

## DPI Helpers

Use `DpiScale` when you need to convert between window coordinates and
framebuffer pixels:

```cpp
cwin::DpiScale dpi = window.getDpiScale();

auto [framebufferX, framebufferY] =
    dpi.windowToFramebuffer(mouseX, mouseY);

auto [framebufferWidth, framebufferHeight] =
    dpi.windowSizeToFramebuffer(uiWidth, uiHeight);
```

`Window::getDpiScale()` uses the window content scale. `WindowContext` also
offers `getDpiScale(monitorId)` for monitor-level conversions.

## Vulkan Windows

For Vulkan, create a window with `.noGraphicsApi()`.

```cpp
auto window = cwin::WindowBuilder{}
                  .title("Vulkan App")
                  .size(1280, 720)
                  .noGraphicsApi()
                  .resizable()
                  .build();
```

Query required instance extensions from the context:

```cpp
auto extensions = cwin::WindowContext::get().getRequiredVulkanInstanceExtensions();
```

After creating the Vulkan instance, create the window surface:

```cpp
cwin::VulkanHandle surface = window.createVulkanSurface(instance);
```

`VulkanHandle` is an integer handle type so the public API can avoid including
Vulkan headers.

## Input

Input state is per-window.

```cpp
const auto& input = window.getInput();

if (input.isKeyPressed(cwin::Key::Escape)) {
    window.requestClose();
}

if (input.isMouseButtonDown(cwin::MouseButton::Left)) {
    auto [x, y] = input.getMousePosition();
}
```

Available input queries:

- `isKeyDown`, `isKeyPressed`, `isKeyReleased`
- `isMouseButtonDown`, `isMouseButtonPressed`, `isMouseButtonReleased`
- `getMousePosition`
- `getMouseDelta`
- `getScrollDelta`
- `isMouseInside`

`Pressed` and `Released` are frame transitions. Poll once per frame before
reading them. `getMouseDelta()` and `getScrollDelta()` are also per-frame
values and reset on the next `pollEvents()`.

Mouse control helpers live on `Window`:

```cpp
window.setMousePosition(640.0, 360.0);

if (cwin::WindowContext::get().isRawMouseMotionSupported()) {
    window.setCursorMode(cwin::CursorMode::Captured);
    window.setRawMouseMotion(true);
}
```

Raw mouse motion is useful for camera controls. It is only effective while the
cursor is captured on platforms that support raw motion.

For named commands, use `ActionMap`:

```cpp
cwin::ActionMap actions;
const cwin::ActionId jump = actions.getOrCreateActionId("jump");
const cwin::ActionId saveAction = actions.getOrCreateActionId("save");
const cwin::ActionId leftDash = actions.getOrCreateActionId("left_dash");
const cwin::ActionId fire = actions.getOrCreateActionId("fire");
const cwin::ActionId moveX = actions.getOrCreateActionId("move_x");

actions.bindKey(jump, cwin::Key::Space)
       .bindKey(saveAction, cwin::Key::S, cwin::Modifiers{ .control = true })
       .bindKeyCombo(leftDash, cwin::Key::A, { cwin::Key::LShift })
       .bindMouseButton(fire, cwin::MouseButton::Left)
       .bindGamepadButton(jump, cwin::GamepadButton::A)
       .bindGamepadAxis(moveX, cwin::GamepadAxis::LeftX, 0.20f)
       .setContext(jump, "gameplay")
       .setContext(fire, "gameplay")
       .setContext(moveX, "gameplay");

while (!window.shouldClose()) {
    ctx.pollEvents();
    actions.update(window.getInput(), ctx.getGamepadState(0));

    if (actions.isPressed(jump)) {
        jump();
    }

    if (actions.isPressed(saveAction)) {
        save();
    }

    move(actions.getAxis(moveX));
}
```

Use action names at setup or rebinding-screen boundaries, then keep the
returned `ActionId` values for per-frame queries. `getActions()` returns
snapshots with names, metadata, bindings, and current state for debug UI or
rebinding menus.

Use input contexts:

```cpp
actions.setContextEnabled("gameplay", !menuOpen);
```

For rebinding screens, replace one binding category without rebuilding the
whole map:

```cpp
actions.replaceKey(jump, cwin::Key::J);
actions.replaceKeyCombo(leftDash, cwin::Key::A, { cwin::Key::LShift });
actions.replaceGamepadAxis(moveX, cwin::GamepadAxis::LeftX, 0.25f);
```

## Events

Events are value objects stored by each window for the current frame. Use
`is<T>()`, `getIf<T>()`, or `visit()`.

```cpp
for (const auto& event : window.events()) {
    if (const auto* resized = event.getIf<cwin::Event::FramebufferResized>()) {
        resizeRenderTargets(resized->width, resized->height);
    }

    if (const auto* scale = event.getIf<cwin::Event::ContentScaleChanged>()) {
        updateUiScale(scale->xScale, scale->yScale);
    }
}
```

For application code, `EventDispatcher` is usually cleaner. Register handlers
once, then dispatch the current frame's event span after polling:

```cpp
cwin::EventDispatcher dispatcher;
dispatcher
    .on<cwin::Event::Closed>([&] {
        window.requestClose();
    })
    .on<cwin::Event::KeyPressed>([&](const cwin::Event::KeyPressed& key) {
        if (key.key == cwin::Key::Escape) {
            window.requestClose();
        }
    });

while (!window.shouldClose()) {
    ctx.pollEvents();
    dispatcher.dispatch(window.events());
}
```

Use `each()` when you want every raw event, such as for logging:

```cpp
cwin::EventDispatcher dispatcher;
dispatcher.each([](const cwin::Event& event) {
    event.visit([](const auto& payload) {
        log(payload);
    });
});
```

Use `subscribe()` or `subscribeEach()` when you need to disconnect a handler,
for example when closing a modal tool or disabling a temporary input layer:

```cpp
cwin::EventDispatcher dispatcher;

auto keyHandler =
    dispatcher.subscribe<cwin::Event::KeyPressed>([](const cwin::Event::KeyPressed& key) {
        handleTemporaryShortcut(key);
    });

dispatcher.disconnect(keyHandler);
```

Common event types:

- `Closed`
- `Resized`
- `FramebufferResized`
- `Moved`
- `Minimized`, `Restored`, `Maximized`
- `ContentScaleChanged`
- `MonitorChanged`
- `FocusLost`, `FocusGained`
- `TextEntered`
- `KeyPressed`, `KeyReleased`
- `MouseMoved`, `MouseEntered`, `MouseLeft`
- `MouseButtonPressed`, `MouseButtonReleased`
- `MouseWheelScrolled`
- `GamepadConnected`, `GamepadDisconnected`
- `GamepadButtonPressed`, `GamepadButtonReleased`
- `GamepadAxisMoved`
- `JoystickConnected`, `JoystickDisconnected`
- `JoystickButtonPressed`, `JoystickButtonReleased`
- `JoystickMoved`
- `FilesDropped`

`TextEntered` reports Unicode code points after keyboard layout and input
method processing. Use it for text fields, and use `KeyPressed` for commands
and shortcuts.

Keyboard and mouse button events carry a `Modifiers` value:

```cpp
if (const auto* key = event.getIf<cwin::Event::KeyPressed>()) {
    if (key->key == cwin::Key::S && key->modifiers.control) {
        save();
    }
}
```

## Gamepads

CppWindow exposes standard-mapped gamepads through GLFW's gamepad mapping
layer. This gives stable button and axis names across common controllers.

```cpp
auto& ctx = cwin::WindowContext::get();

for (const auto& gamepad : ctx.getGamepads()) {
    std::cout << gamepad.id << ": " << gamepad.name << "\n";
}

if (auto state = ctx.getGamepadState(0)) {
    if (state->isButtonDown(cwin::GamepadButton::A)) {
        jump();
    }

    float moveX = state->getAxis(cwin::GamepadAxis::LeftX);
    float moveY = state->getAxis(cwin::GamepadAxis::LeftY);
}
```

`getGamepads()` lists present joystick devices and tells you whether each one
has a standard mapping. `getGamepadState()` returns a value only for present
devices with a standard mapping.

Gamepad connection, button, and axis events are delivered to window event
queues after `pollEvents()`.

Raw joystick events are also delivered for every present GLFW joystick slot.
Use `Joystick*` events when you need backend button/axis indices, and use
`Gamepad*` events when you want GLFW's standard controller mapping.

## Clipboard And File Drop

Clipboard text is available on `WindowContext`:

```cpp
auto& ctx = cwin::WindowContext::get();

if (!ctx.setClipboardText("Copied from my app")) {
    // Clipboard write was rejected by the platform/backend.
}

if (auto text = ctx.tryGetClipboardText()) {
    paste(*text);
}
```

`tryGetClipboardText()` returns `std::nullopt` when the backend reports a
clipboard read error. `getClipboardText()` is still available as a convenience
fallback and returns an empty string on failure. `hasClipboardText()` reports
whether readable, non-empty clipboard text is currently available.

Drag-and-drop files onto a window to receive `Event::FilesDropped`:

```cpp
for (const auto& event : window.events()) {
    if (const auto* drop = event.getIf<cwin::Event::FilesDropped>()) {
        for (const auto& path : drop->paths) {
            loadFile(path);
        }
    }
}
```

The drop event includes the cursor position in window coordinates.

## Window Controls

Basic controls:

```cpp
window.setTitle("New Title");
window.setSize(1024, 768);
window.setPosition(100, 100);
window.setResizable(true);
window.setDecorated(true);
window.setFloating(false);
window.setOpacity(0.95f);
window.setVisible(true);
window.setFocus(true);

if (window.isResizable() && window.isDecorated()) {
    // Update app UI state.
}
```

State controls:

```cpp
window.minimize();
window.maximize();
window.restore();

if (window.isMinimized() || window.isMaximized()) {
    // Adjust rendering or UI behavior if needed.
}
```

Size constraints:

```cpp
window.setSizeLimits({
    .minWidth = 640,
    .minHeight = 360,
    .maxWidth = 1920,
    .maxHeight = 1080,
});

window.setAspectRatio({ 16, 9 });
```

Clear constraints when they are no longer needed:

```cpp
window.clearSizeLimits();
window.clearAspectRatio();
```

Cursor modes:

```cpp
window.setCursorMode(cwin::CursorMode::Normal);
window.setCursorMode(cwin::CursorMode::Hidden);
window.setCursorMode(cwin::CursorMode::Captured);
```

`Captured` is the mode to use for first-person camera control.

Cursor shapes and custom RGBA cursor images:

```cpp
window.setCursorShape(cwin::CursorShape::Hand);

std::array<uint8_t, 16 * 16 * 4> pixels{};
fillCursorPixels(pixels);
cwin::ImageRgba cursor{
    .width = 16,
    .height = 16,
    .pixels = pixels,
};
window.setCursorImage(cursor, 0, 0);
window.clearCursor();
```

Window icons use the same `ImageRgba` format. Some platforms, including macOS,
may ignore per-window icons.

```cpp
std::array<uint8_t, 32 * 32 * 4> iconPixels{};
fillIconPixels(iconPixels);
cwin::ImageRgba icon{
    .width = 32,
    .height = 32,
    .pixels = iconPixels,
};

window.setIcon(icon);
window.clearIcon();
window.requestAttention();
```

## Fullscreen Modes

CppWindow exposes four modes:

- `WindowMode::Windowed`: normal restored window.
- `WindowMode::Fullscreen`: decorated maximized window with the title bar.
- `WindowMode::BorderlessFullscreen`: undecorated window covering the monitor
  work area. This avoids display mode changes and is the recommended default
  for most apps and games.
- `WindowMode::ExclusiveFullscreen`: true monitor fullscreen through GLFW. This
  can change video mode, framebuffer size, content scale, and OS fullscreen
  behavior.

Example:

```cpp
window.setWindowMode(cwin::WindowMode::Fullscreen);
window.setWindowMode(cwin::WindowMode::BorderlessFullscreen);
window.setWindowMode(cwin::WindowMode::ExclusiveFullscreen);
window.setWindowMode(cwin::WindowMode::Windowed);
```

For exclusive fullscreen on macOS/Retina displays, cursor size and content
scaling can appear to change because the system may change the active display
mode and framebuffer scale. Treat this as platform behavior. Render using
`getFramebufferSize()`, handle `FramebufferResized`, and listen for
`ContentScaleChanged`.

If you want smooth app switching and stable desktop scaling, use
`BorderlessFullscreen`.

## Monitors

Use `WindowContext` to inspect monitors and video modes:

```cpp
auto& ctx = cwin::WindowContext::get();

for (const auto& monitor : ctx.getMonitors()) {
    std::cout << monitor.id << ": " << monitor.name << "\n";
    std::cout << monitor.currentVideoMode.width << "x"
              << monitor.currentVideoMode.height << "\n";
}

auto primary = ctx.getPrimaryMonitor();
auto modes = ctx.getVideoModes(primary ? primary->id : 0);
auto [scaleX, scaleY] = ctx.getContentScale();
cwin::DpiScale dpi = ctx.getDpiScale();
```

Pass a monitor id to `setWindowMode` when you want a specific monitor:

```cpp
window.setWindowMode(cwin::WindowMode::BorderlessFullscreen, monitorId);
```

For exclusive fullscreen, optionally pass a target video mode:

```cpp
cwin::VideoMode mode{
    .width = 1920,
    .height = 1080,
    .redBits = 8,
    .greenBits = 8,
    .blueBits = 8,
    .refreshRate = 60,
};

window.setWindowMode(cwin::WindowMode::ExclusiveFullscreen, monitorId, mode);
```

## Recommended Game Loop Shape

CppWindow intentionally leaves the loop under your control.

```cpp
cwin::FrameTimer timer;
cwin::FpsCounter fps;
cwin::FrameLimiter limiter(60.0);

const bool useVSync = true;
window.setVSync(useVSync);
limiter.setVSyncEnabled(useVSync);

while (!window.shouldClose()) {
    cwin::FrameTime frame = timer.tick();
    if (fps.update(frame)) {
        updateFpsDisplay(fps.framesPerSecond());
    }

    ctx.pollEvents();
    handleEvents(window.events());
    update(frame.deltaSeconds, window.getInput());
    render(window);
    window.swapBuffers(); // OpenGL apps; Vulkan apps present from their renderer.
    limiter.wait();
}
```

`FrameLimiter` does not run the loop. It only sleeps when `wait()` is called.
When VSync is enabled, set `limiter.setVSyncEnabled(true)` so the limiter does
not add a second software delay on top of the presentation wait.

For idle, event-driven apps, replace `pollEvents()` with `waitEvents()` or
`waitEventsTimeout(seconds)`. Use a timeout when animations, timers, or periodic
work still need to progress without incoming input.

For a fixed-step simulation, use `FixedStepAccumulator` in application code:

```cpp
cwin::FrameTimer timer;
cwin::FixedStepAccumulator fixedStep(1.0 / 60.0);

while (!window.shouldClose()) {
    fixedStep.add(timer.tick().deltaSeconds);

    while (fixedStep.consumeStep()) {
        simulate(fixedStep.fixedDeltaSeconds());
    }

    renderInterpolated(fixedStep.alpha());
}
```

`Clock`, `FrameTimer`, `FixedStepAccumulator`, `FpsCounter`, and
`FrameLimiter` do not own or run the loop.

## Native Handles

`window.getNativeHandles()` returns platform handles for interop:

```cpp
cwin::NativeHandles handles = window.getNativeHandles();
```

`handles.system` identifies the platform. The `window` and `display` pointers
are backend/platform-specific and should only be used at integration boundaries.

## Error Handling

CppWindow throws `cwin::Error` for unrecoverable public API failures, such as
backend initialization failure, native window creation failure, or Vulkan
surface creation failure.

```cpp
try {
    auto& ctx = cwin::WindowContext::get();
    auto window = cwin::WindowBuilder{}.title("App").noGraphicsApi().build();
} catch (const cwin::Error& error) {
    std::cerr << error.what() << "\n";
}
```

`cwin::Error::code()` returns an `ErrorCode` category. Backend details, such as
GLFW error code and description, are included in `what()` when available.

## Examples

- `examples/basic.cpp`: minimal no-API window.
- `examples/opengl.cpp`: OpenGL setup and presentation.
- `examples/window_controls.cpp`: resizing, decoration, opacity, cursor modes,
  cursor images, icons, attention requests, monitor info, and content-scale events.
- `examples/fullscreen_toggle.cpp`: windowed, fullscreen, borderless
  fullscreen, and exclusive fullscreen modes.
- `examples/mouse_capture.cpp`: captured cursor behavior with an OpenGL status bar.
- `examples/text_input.cpp`: Unicode text input events.
- `examples/gamepad.cpp`: standard gamepad queries and events.
- `examples/app_utilities.cpp`: clipboard, file drop events, FPS, and frame limiting.
- `examples/input_helpers.cpp`: action bindings, mouse positioning, and raw mouse mode.
- `examples/multi_window.cpp`: independent event/input handling for multiple windows.
- `examples/event_viewer.cpp`: logs all event payloads with `Event::visit`.
- `examples/monitor_info.cpp`: monitor metadata, content scale, and video modes.
- `examples/native_handles.cpp`: platform handle and Vulkan extension inspection.
- `examples/fixed_step_loop.cpp`: caller-owned loop with fixed-step simulation,
  FPS sampling, and frame limiting.
- `examples/particles.cpp`: richer OpenGL rendering example.

For short, copyable patterns that combine these APIs, see
the [Recipes](recipes.md).
