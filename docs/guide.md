# CppWindow Guide

CppWindow is a C++20 windowing and input layer built on GLFW. It is intended
to be the platform layer under a game, tool, renderer, or small graphics app.
It owns windows, event collection, per-window input state, monitor queries, and
native surface/context hooks.

CppWindow does not own the application loop and does not provide a renderer.
Your application decides when to poll, update, render, and present.

For a complete symbol-by-symbol reference, see the [API Reference](api.md).

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
cmake -S . -B build -DCPPWINDOW_BUILD_EXAMPLES=ON -DCPPWINDOW_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
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
cmake -S . -B build-docs -DCPPWINDOW_BUILD_DOCS=ON -DCPPWINDOW_BUILD_EXAMPLES=OFF -DCPPWINDOW_BUILD_TESTS=OFF
cmake --build build-docs --target cppwindow_docs
```

## Minimal Window

Use `WindowBuilder` to create a window. Use `WindowContext::Get()` to poll
platform events.

```cpp
#include <cppwindow/cppwindow.hpp>

int main()
{
    auto& ctx = cwin::WindowContext::Get();

    auto window = cwin::WindowBuilder{}
                      .title("CppWindow App")
                      .size(1280, 720)
                      .noAPI()
                      .resizable()
                      .build();

    while (!window.shouldClose()) {
        ctx.pollEvents();

        for (const auto& event : window.events()) {
            if (event.is<cwin::Event::Closed>()) {
                window.requestClose();
            }
        }

        // Update and render here.
    }
}
```

`pollEvents()` clears each window event queue, updates previous input state,
then collects new events for the current frame. Read `window.events()` and
`window.getInput()` after polling.

Builder options can cover most initial window state:

```cpp
auto window = cwin::WindowBuilder{}
                  .title("Configured App")
                  .size(1280, 720)
                  .position(120, 80)
                  .noAPI()
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
auto& ctx = cwin::WindowContext::Get();

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

    auto [fbWidth, fbHeight] = window.getFrameBufferSize();
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

For Vulkan, create a window with `.noAPI()`.

```cpp
auto window = cwin::WindowBuilder{}
                  .title("Vulkan App")
                  .size(1280, 720)
                  .noAPI()
                  .resizable()
                  .build();
```

Query required instance extensions from the context:

```cpp
auto extensions = cwin::WindowContext::Get().getRequiredGlfwVulkanExtensions();
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

if (cwin::WindowContext::Get().isRawMouseMotionSupported()) {
    window.setCursorMode(cwin::CursorMode::Captured);
    window.setRawMouseMotion(true);
}
```

Raw mouse motion is useful for camera controls. It is only effective while the
cursor is captured on platforms that support raw motion.

For named commands, use `ActionMap`:

```cpp
cwin::ActionMap actions;
actions.bindKey("jump", cwin::Key::Space)
       .bindMouseButton("fire", cwin::MouseButton::Left)
       .bindGamepadButton("jump", cwin::GamepadButton::A);

while (!window.shouldClose()) {
    ctx.pollEvents();
    actions.update(window.getInput(), ctx.getGamepadState(0));

    if (actions.isPressed("jump")) {
        jump();
    }
}
```

## Events

Events are value objects stored by each window for the current frame. Use
`is<T>()`, `getIf<T>()`, or `visit()`.

```cpp
for (const auto& event : window.events()) {
    if (const auto* resized = event.getIf<cwin::Event::FrameBufferResized>()) {
        resizeRenderTargets(resized->width, resized->height);
    }

    if (const auto* scale = event.getIf<cwin::Event::ContentScaleChanged>()) {
        updateUiScale(scale->xScale, scale->yScale);
    }
}
```

Common event types:

- `Closed`
- `Resized`
- `FrameBufferResized`
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
auto& ctx = cwin::WindowContext::Get();

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
auto& ctx = cwin::WindowContext::Get();

ctx.setClipboardText("Copied from my app");
std::string pasted = ctx.getClipboardText();
```

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
```

State controls:

```cpp
window.minimize();
window.maximize();
window.restore();
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
`getFrameBufferSize()`, handle `FrameBufferResized`, and listen for
`ContentScaleChanged`.

If you want smooth app switching and stable desktop scaling, use
`BorderlessFullscreen`.

## Monitors

Use `WindowContext` to inspect monitors and video modes:

```cpp
auto& ctx = cwin::WindowContext::Get();

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
    auto& ctx = cwin::WindowContext::Get();
    auto window = cwin::WindowBuilder{}.title("App").noAPI().build();
} catch (const cwin::Error& error) {
    std::cerr << error.what() << "\n";
}
```

`cwin::Error::code()` returns an `ErrorCode` category. Backend details, such as
GLFW error code and description, are included in `what()` when available.

## Examples

- `examples/basic.cpp`: minimal no-API window.
- `examples/opengl.cpp`: OpenGL setup and presentation.
- `examples/window_controls.cpp`: resizing, decoration, opacity, cursor mode,
  monitor info, and content-scale events.
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
