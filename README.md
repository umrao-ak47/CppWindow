# CppWindow

CppWindow is a modern C++20 windowing and input abstraction built on top of GLFW, designed for high-performance graphics applications and engine development.

It provides a clean, type-safe API for window creation, event processing, and
per-window input while keeping backend details out of public headers.

CppWindow is not a rendering library - it is a platform layer intended to sit beneath Vulkan, OpenGL, or future graphics backends.

## Project Status

CppWindow is pre-1.0 software. It is usable for experiments and early
applications, but the public API may still change while real projects exercise
the design.

Use the `main` branch for the supported GLFW backend. The `native-backend`
branch is experimental and exists for learning/native backend exploration; do
not treat it as the recommended integration branch.

## Documentation

- [CppWindow Guide](docs/guide.md): setup, event loop, OpenGL/Vulkan usage, input, events, monitors, window controls, and fullscreen behavior.
- [Recipes](docs/recipes.md): copyable game loop, tool loop, action context, multi-window, and fullscreen patterns.
- [API Reference](docs/api.md): generated public API reference from public headers.
- [Release Process](docs/release.md): manual release workflow and artifacts.

## ✨ Features

- Modern C++20 design (RAII, spans, variants, concepts)
- Builder-based window creation
- Per-window input state
- Strongly typed event system
- Structured modifier state and CppWindow error categories
- Vulkan surface support
- OpenGL context support
- Window controls, waitable event loops, and monitor queries
- Mouse delta, text input, raw joystick events, and standard gamepad support
- Action bindings, mouse positioning, and raw mouse motion controls
- Clipboard status, file drop events, DPI conversion, FPS, and frame pacing helpers
- Optional Dear ImGui core target, platform backend, and renderer-agnostic layer
- Zero global input state
- Backend abstraction (currently GLFW)
- Minimal runtime overhead

## 📦 Requirements

- **CMake**: 3.20 or higher
- **C++ Compiler**: Support for C++20 (e.g., GCC 11+, Clang 13+, MSVC 19.29+)
- **Dependencies**: GLFW 3.4 (automatically managed via `FetchContent`)
- **Optional Dependencies**: Dear ImGui when `CPPWINDOW_BUILD_IMGUI=ON`

## 🛠 Integration

### Git Submodule

```bash
git submodule add git@github.com:umrao-ak47/CppWindow.git external/CppWindow
```

```cmake
add_subdirectory(external/CppWindow)
target_link_libraries(your_project PRIVATE cppwindow::cppwindow)
```

If your project already defines a `glfw` target, CppWindow uses it. Otherwise
CppWindow fetches GLFW 3.4 with CMake `FetchContent`.

### Installed Package

```cmake
find_package(cppwindow CONFIG REQUIRED)
target_link_libraries(your_project PRIVATE cppwindow::cppwindow)
```

CppWindow is currently a static library. GLFW is private to the public C++ API,
but the installed static target still has a GLFW link dependency. If CppWindow
fetched GLFW during install, GLFW is installed into the same prefix.

### Local Development

For local development with multiple CMake build directories, CMake
`FetchContent` manages dependency download locations. Each build directory
compiles dependencies with its own flags.

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

Optional Dear ImGui integration is disabled by default. Enable it when you want
the reusable ImGui target, CppWindow platform backend, and renderer-agnostic
layer:

```bash
cmake --preset imgui
cmake --build --preset imgui
ctest --preset imgui
```

Apps link `cppwindow::imgui`; extensions such as ImPlot link
`cppwindow::dear_imgui` so the app and extensions share the same ImGui context
and configuration. CppWindow can fetch Dear ImGui, or you can provide an
existing target with `CPPWINDOW_DEAR_IMGUI_TARGET`. See the
[guide](docs/guide.md#dear-imgui) for renderer adapter and extension examples.
Example targets include `cppwindow_example_imgui_minimal`,
`cppwindow_example_imgui_overlay`, `cppwindow_example_imgui_input_capture`, and
`cppwindow_example_imgui_style_demo`.

## 🚀 Quick Start

### Window Creation

```cpp
#include <cppwindow/cppwindow.hpp>

int main()
{
    auto& ctx = cwin::Context::get();

    cwin::Window window =
        cwin::WindowBuilder{}
            .title("Basic Example")
            .size(1280, 720)
            .noGraphicsApi()
            .build();

    cwin::EventDispatcher dispatcher;
    dispatcher.on<cwin::Event::Closed>([&]
    {
        window.requestClose();
    });

    while (!window.shouldClose())
    {
        ctx.pollEvents();
        dispatcher.dispatch(window.events());
        dispatcher.dispatch(ctx.events());
    }
}
```

### 🧱 Graphics Modes

CppWindow supports two creation paths:

#### No API (Recommended for Vulkan)

```cpp
auto window =
    cwin::WindowBuilder{}
        .noGraphicsApi()
        .build();
```

Create the surface later:

```cpp
cwin::VulkanHandle surface =
    window.createVulkanSurface(instance);
```

#### OpenGL

CppWindow creates the context, but you must load OpenGL functions.

Example using GLAD:

```cpp
auto& ctx = cwin::Context::get();

auto window =
    cwin::WindowBuilder{}
        .openGL({4,5,true})
        .build();

window.makeContextCurrent();
gladLoadGLLoader(reinterpret_cast<GLADloadproc>(ctx.procLoader()));
```

Render loop:

```cpp
while (!window.shouldClose())
{
    cwin::Context::get().pollEvents();

    glClearColor(0.f,0.f,0.f,1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    window.swapBuffers();
}
```

### 🎮 Input

Input is tracked per window, avoiding hidden global state.

```cpp
const auto& input = window.input();

if (input.isKeyPressed(cwin::Key::Escape))
    window.requestClose();

auto [x,y] = input.mousePosition();
```

### 📬 Events

Events are stored internally and exposed as a span to avoid allocations.

```cpp
cwin::EventDispatcher dispatcher;
dispatcher
    .on<cwin::Event::Resized>([](const cwin::Event::Resized& resized) {
        resize(resized.width, resized.height);
    })
    .each([](const cwin::Event& event) {
        event.visit([](const auto& payload) {
            // optional generic logging
        });
    });

while (!window.shouldClose())
{
    ctx.pollEvents();
    dispatcher.dispatch(window.events());
    dispatcher.dispatch(ctx.events());
}
```

Use `subscribe()` or `subscribeEach()` when a handler should be removed later:

```cpp
auto resizeHandler =
    dispatcher.subscribe<cwin::Event::Resized>([](const cwin::Event::Resized& resized) {
        resize(resized.width, resized.height);
    });

dispatcher.disconnect(resizeHandler);
```

### Window Controls

```cpp
window.setResizable(true);
window.setDecorated(false);
window.setOpacity(0.9f);
window.setCursorMode(cwin::CursorMode::Captured);
window.setCursorShape(cwin::CursorShape::Hand);
window.requestAttention();
window.setWindowMode(cwin::WindowMode::BorderlessFullscreen);
auto placement = window.windowedPlacement();
window.setWindowedPlacement(placement);
std::string title = window.title();
bool decorated = window.isDecorated();
bool maximized = window.isMaximized();
```

See the [CppWindow Guide](docs/guide.md) for fullscreen mode semantics,
high-DPI notes, and monitor APIs. See the [API Reference](docs/api.md) for all
public types and methods.

## ⚖️ License

Licensed under the MIT License. See the [LICENSE](LICENSE) file for full text.
Third-party dependency notices are listed in
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
