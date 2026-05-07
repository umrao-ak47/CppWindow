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

If the parent project already provides a target named `glfw`, CppWindow uses
that target. Otherwise it fetches GLFW 3.4 with CMake `FetchContent`.

Then include the public header:

```cpp
#include <cppwindow/cppwindow.hpp>
```

CppWindow currently builds a static library target named
`cppwindow::cppwindow`. GLFW is private to CppWindow's public C++ API, but a
static `libcppwindow.a` still needs GLFW at final link time. Installed packages
therefore call `find_dependency(glfw3)` and expose GLFW as a link dependency of
the imported CppWindow target. When CppWindow fetched GLFW during installation,
GLFW is installed into the same prefix for convenience.

Installed package usage:

```cmake
find_package(cppwindow CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE cppwindow::cppwindow)
```

## API Shape

Public methods use `lowerCamelCase`. State queries use noun-style accessors
such as `window.input()`, `window.framebufferSize()`, `ctx.monitors()`, and
`actions.binding(id)`. Boolean state uses `isX()` or `hasX()`. The main
exceptions are `WindowContext::get()` for the singleton context and
`Event::getIf<T>()`, which follows the `std::get_if` naming pattern.

## Build This Repository

Configure and build:

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

By default, CMake `FetchContent` stores dependencies under each build
directory. CppWindow keeps that behavior, so each build directory manages and
compiles dependencies with its own flags.

```bash
cmake --preset sanitizers
cmake --build --preset sanitizers
ctest --preset sanitizers

cmake --preset docs
cmake --build --preset docs

cmake --preset imgui
cmake --build --preset imgui
ctest --preset imgui

cmake --preset install
cmake --build --preset install

cmake --preset install-imgui
cmake --build --preset install-imgui

cd tests/package_consumer
cmake --preset installed
cmake --build --preset installed
ctest --preset installed

cmake --preset installed-imgui
cmake --build --preset installed-imgui
ctest --preset installed-imgui

cd ../subproject_consumer
cmake --preset subproject
cmake --build --preset subproject
ctest --preset subproject

cmake --preset provided-glfw
cmake --build --preset provided-glfw
ctest --preset provided-glfw
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
cmake --build --preset dev --target cppwindow_example_basic
cmake --build --preset dev --target cppwindow_example_opengl
cmake --build --preset dev --target cppwindow_example_window_controls
cmake --build --preset dev --target cppwindow_example_fullscreen_toggle
cmake --build --preset dev --target cppwindow_example_input_helpers
cmake --build --preset dev --target cppwindow_example_app_utilities
cmake --build --preset dev --target cppwindow_example_particles
cmake --build --preset imgui --target cppwindow_example_imgui_minimal
cmake --build --preset imgui --target cppwindow_example_imgui_overlay
cmake --build --preset imgui --target cppwindow_example_imgui_input_capture
cmake --build --preset imgui --target cppwindow_example_imgui_style_demo
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
`window.input()` after polling.

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
gladLoadGLLoader(reinterpret_cast<GLADloadproc>(ctx.procLoader()));
window.setVSync(true);

while (!window.shouldClose()) {
    ctx.pollEvents();

    auto [fbWidth, fbHeight] = window.framebufferSize();
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
cwin::DpiScale dpi = window.dpiScale();

auto [framebufferX, framebufferY] =
    dpi.windowToFramebuffer(mouseX, mouseY);

auto [framebufferWidth, framebufferHeight] =
    dpi.windowSizeToFramebuffer(uiWidth, uiHeight);
```

`Window::dpiScale()` uses the window content scale. `WindowContext` also
offers `dpiScale(monitorId)` for monitor-level conversions.

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
auto extensions = cwin::WindowContext::get().requiredVulkanInstanceExtensions();
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
const auto& input = window.input();

if (input.isKeyPressed(cwin::Key::Escape)) {
    window.requestClose();
}

if (input.isMouseButtonDown(cwin::MouseButton::Left)) {
    auto [x, y] = input.mousePosition();
}
```

Available input queries:

- `isKeyDown`, `isKeyPressed`, `isKeyReleased`
- `isMouseButtonDown`, `isMouseButtonPressed`, `isMouseButtonReleased`
- `mousePosition`
- `mouseDelta`
- `scrollDelta`
- `isMouseInside`

`Pressed` and `Released` are frame transitions. Poll once per frame before
reading them. `mouseDelta()` and `scrollDelta()` are also per-frame
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
const cwin::ActionId jump = actions.defineAction("jump");
const cwin::ActionId saveAction = actions.defineAction("save");
const cwin::ActionId leftDash = actions.defineAction("left_dash");
const cwin::ActionId fire = actions.defineAction("fire");
const cwin::ActionId moveX = actions.defineAction("move_x");

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
    actions.update(window.input(), ctx.gamepadState(0));

    if (actions.isPressed(jump)) {
        jump();
    }

    if (actions.isPressed(saveAction)) {
        save();
    }

    move(actions.axisValue(moveX));
}
```

Use action names at setup or rebinding-screen boundaries, then keep the
returned `ActionId` values for per-frame queries. `actions()` returns
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

## Dear ImGui

CppWindow's ImGui support is optional and renderer-agnostic. The integration
provides a reusable Dear ImGui target and a CppWindow platform backend, but the
app owns the renderer backend.

- `cppwindow::dear_imgui`: builds Dear ImGui and exposes its headers.
- `cppwindow::imgui`: feeds CppWindow events, clipboard, cursors, DPI, and
  display size into ImGui.
- `cwin::imgui::Context`: owns a Dear ImGui context with RAII lifetime.
- `cwin::imgui::Layer`: coordinates the common platform + renderer frame flow
  with a renderer adapter supplied by the app.

The optional headers live outside CppWindow's base include tree and are exposed
only by the ImGui targets. Include the umbrella header when you use the
integration:

```cpp
#include <cppwindow/imgui.hpp>
```

### Build Integration

Enable the targets when CppWindow is used as a subproject:

```cmake
set(CPPWINDOW_BUILD_IMGUI ON)
add_subdirectory(external/CppWindow)

target_link_libraries(app PRIVATE
    cppwindow::imgui)
```

When `CPPWINDOW_DEAR_IMGUI_TARGET` is empty, CppWindow fetches the pinned Dear
ImGui version with CMake `FetchContent` and builds `cppwindow::dear_imgui`.

```cmake
set(CPPWINDOW_BUILD_IMGUI ON)
add_subdirectory(external/CppWindow)
```

When the app already owns Dear ImGui, provide a target instead. This is the
path for custom `imconfig.h`, docking branches, or a shared engine-level ImGui
build:

```cmake
add_library(app_imgui STATIC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_demo.cpp)
target_include_directories(app_imgui PUBLIC external/imgui)

set(CPPWINDOW_BUILD_IMGUI ON)
set(CPPWINDOW_DEAR_IMGUI_TARGET app_imgui)
add_subdirectory(external/CppWindow)

target_link_libraries(app PRIVATE cppwindow::imgui)
```

When consuming an installed package:

```cmake
find_package(cppwindow CONFIG REQUIRED COMPONENTS imgui)
target_link_libraries(app PRIVATE cppwindow::imgui)
```

Package builds use CppWindow's fetched `cppwindow::dear_imgui` target. A
user-provided `CPPWINDOW_DEAR_IMGUI_TARGET` is intended for `add_subdirectory`
builds, because CppWindow cannot export an arbitrary app-owned target in its
installed package config.

Build the included ImGui examples with:

```bash
cmake --preset imgui
cmake --build --preset imgui --target cppwindow_example_imgui_minimal
cmake --build --preset imgui --target cppwindow_example_imgui_overlay
cmake --build --preset imgui --target cppwindow_example_imgui_input_capture
cmake --build --preset imgui --target cppwindow_example_imgui_style_demo
```

### Extensions

Extensions should link to the same Dear ImGui target as the app. This keeps
ImPlot, ImGuizmo, or other add-ons on the same `imgui.h`, `imconfig.h`, compile
definitions, and context:

```cmake
add_library(implot
    external/implot/implot.cpp
    external/implot/implot_items.cpp)
target_include_directories(implot PUBLIC external/implot)
target_link_libraries(implot PUBLIC cppwindow::dear_imgui)

target_link_libraries(app PRIVATE
    cppwindow::imgui
    implot)
```

Do not compile a second copy of Dear ImGui for extensions. `cppwindow::imgui`
and every extension should share `cppwindow::dear_imgui`. In user-target mode,
`cppwindow::dear_imgui` forwards to `CPPWINDOW_DEAR_IMGUI_TARGET`.

### Platform Backend Only

Use `cwin::imgui::Platform` directly when your app already has its own renderer
frame flow:

```cpp
#include <cppwindow/imgui.hpp>

cwin::imgui::Context imguiContext;
cwin::imgui::Platform imguiPlatform(window);

ctx.pollEvents();
imguiPlatform.handleEvents(window.events());
imguiPlatform.newFrame();
ImGui::NewFrame();

ImGui::Begin("Tools");
ImGui::Text("Renderer owned by the app");
ImGui::End();

ImGui::Render();
myRenderer.render(ImGui::GetDrawData());
```

### Layer With App-Owned Renderer

Use `cwin::imgui::Layer` when you want CppWindow to coordinate the common
platform + renderer frame calls. The renderer adapter can wrap Dear ImGui's
official OpenGL, Vulkan, Metal, or DX backends, or a renderer owned by your
engine:

```cpp
#include <cppwindow/imgui.hpp>

class MyImGuiRenderer {
public:
    void newFrame();
    void render(ImDrawData* drawData);
};

cwin::imgui::Context imguiContext{ { .style = cwin::imgui::Style::Dark } };
{
    cwin::imgui::Layer<MyImGuiRenderer> imguiLayer(window);

    while (!window.shouldClose()) {
        ctx.pollEvents();
        imguiLayer.handleEvents(window.events());
        imguiLayer.newFrame();

        ImGui::Begin("Debug");
        ImGui::Text("Hello");
        ImGui::End();

        renderScene();
        imguiLayer.render();
        window.swapBuffers();
    }
}
```

The renderer adapter only needs:

```cpp
void newFrame();
void render(ImDrawData* drawData);
```

### OpenGL Adapter Example

The library does not export an OpenGL ImGui renderer target. The included ImGui
examples provide a local OpenGL renderer adapter in
`extras/imgui/examples/opengl_imgui_renderer.cpp`. It uploads ImGui texture
requests, owns the OpenGL shader, buffers, and vertex array, and renders
`ImDrawData` directly. The adapter requires OpenGL 3.3 or newer; the examples
request OpenGL 4.1 core for macOS compatibility:

```cpp
#include "opengl_imgui_renderer.hpp"

example::loadOpenGL(ctx);
cwin::imgui::Layer<example::OpenGLImGuiRenderer> imguiLayer(window, "#version 410");
```

This adapter is intentionally example-only. Applications can copy that pattern
for OpenGL, or provide a different renderer adapter for Vulkan, Metal, DirectX,
or an engine renderer.

### Input Capture

Disable gameplay input contexts while ImGui wants mouse or keyboard input:

```cpp
imguiLayer.newFrame();

actions.setContextEnabled(
    "gameplay",
    !imguiLayer.wantsMouse() && !imguiLayer.wantsKeyboard());

actions.update(window.input());
```

Use `wantsTextInput()` when text entry should disable additional shortcuts:

```cpp
actions.setContextEnabled("shortcuts", !imguiLayer.wantsTextInput());
```

Create the ImGui context before constructing `Platform` or `Layer`, and destroy
it after those objects have been destroyed.

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
- `Refresh`
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

Use `WindowContext::keyName(key, scancode)` when UI needs the platform/localized
label for a key, and `keyScancode(key)` when storing platform scancode-based
bindings.

## Gamepads

CppWindow exposes standard-mapped gamepads through GLFW's gamepad mapping
layer. This gives stable button and axis names across common controllers.

```cpp
auto& ctx = cwin::WindowContext::get();

for (const auto& gamepad : ctx.gamepads()) {
    std::cout << gamepad.id << ": " << gamepad.name << "\n";
}

if (auto state = ctx.gamepadState(0)) {
    if (state->isButtonDown(cwin::GamepadButton::A)) {
        jump();
    }

    float moveX = state->axis(cwin::GamepadAxis::LeftX);
    float moveY = state->axis(cwin::GamepadAxis::LeftY);
}
```

`gamepads()` lists present joystick devices and tells you whether each one
has a standard mapping. `gamepadState()` returns a value only for present
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

if (auto text = ctx.clipboardText()) {
    paste(*text);
}
```

`clipboardText()` returns `std::nullopt` when the backend reports a clipboard
read error. `hasClipboardText()` reports whether readable, non-empty clipboard
text is currently available.

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
std::string title = window.title();
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

Persist restored/windowed placement separately from fullscreen state:

```cpp
cwin::WindowPlacement saved = window.windowedPlacement();
window.setWindowedPlacement(saved);
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
`framebufferSize()`, handle `FramebufferResized`, and listen for
`ContentScaleChanged`.

If you want smooth app switching and stable desktop scaling, use
`BorderlessFullscreen`.

## Monitors

Use `WindowContext` to inspect monitors and video modes:

```cpp
auto& ctx = cwin::WindowContext::get();

for (const auto& monitor : ctx.monitors()) {
    std::cout << monitor.id << ": " << monitor.name << "\n";
    std::cout << monitor.currentVideoMode.width << "x"
              << monitor.currentVideoMode.height << "\n";
}

auto primary = ctx.primaryMonitor();
auto modes = ctx.videoModes(primary ? primary->id : 0);
auto [scaleX, scaleY] = ctx.contentScale();
cwin::DpiScale dpi = ctx.dpiScale();
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
    update(frame.deltaSeconds, window.input());
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

`window.nativeHandles()` returns platform handles for interop:

```cpp
cwin::NativeHandles handles = window.nativeHandles();
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

Core and loop examples:

- `examples/basic.cpp`: minimal no-API window using `waitEventsTimeout()`.
- `examples/fixed_step_loop.cpp`: caller-owned loop with fixed-step simulation,
  FPS sampling, and frame limiting.
- `examples/multi_window.cpp`: independent event/input handling for multiple windows.
- `examples/event_viewer.cpp`: logs all event payloads with `Event::visit`.

Window, platform, and input examples:

- `examples/window_controls.cpp`: resizing, decoration, opacity, cursor modes,
  cursor images, icons, attention requests, monitor info, and content-scale events.
- `examples/fullscreen_toggle.cpp`: windowed, fullscreen, borderless
  fullscreen, and exclusive fullscreen modes.
- `examples/borderless.cpp`: initially undecorated OpenGL window.
- `examples/mouse_capture.cpp`: captured cursor behavior with an OpenGL status bar.
- `examples/text_input.cpp`: Unicode text input events.
- `examples/gamepad.cpp`: standard gamepad queries and events.
- `examples/app_utilities.cpp`: clipboard, file drop events, FPS, and frame limiting.
- `examples/input_helpers.cpp`: action bindings, mouse positioning, and raw mouse mode.
- `examples/monitor_info.cpp`: monitor metadata, content scale, and video modes.
- `examples/native_handles.cpp`: platform handle and Vulkan extension inspection.

OpenGL rendering examples:

- `examples/opengl.cpp`: minimal OpenGL setup and presentation.
- `examples/heightmap.cpp`: simple 3D terrain rendering.
- `examples/particles.cpp`: richer OpenGL particle rendering.

ImGui examples:

- `extras/imgui/examples/imgui_minimal.cpp`: smallest ImGui setup with
  `cwin::imgui::Layer`.
- `extras/imgui/examples/imgui_overlay.cpp`: optional Dear ImGui layer with an
  app-owned OpenGL renderer adapter.
- `extras/imgui/examples/imgui_input_capture.cpp`: ImGui capture state with
  action contexts.
- `extras/imgui/examples/imgui_style_demo.cpp`: style editor and Dear ImGui
  demo window.

For short, copyable patterns that combine these APIs, see
the [Recipes](recipes.md).
