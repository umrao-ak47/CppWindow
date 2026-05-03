# CppWindow

CppWindow is a modern C++20 windowing and input abstraction built on top of GLFW, designed for high-performance graphics applications and engine development.

It provides a clean, type-safe API for window creation, event processing, and per-window input while keeping backend details fully hidden.

CppWindow is not a rendering library - it is a platform layer intended to sit beneath Vulkan, OpenGL, or future graphics backends.

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

#### Add the submodule to your project:

```bash
git submodule add git@github.com:umrao-ak47/CppWindow.git external/CppWindow
```

#### Include it in your CMakeLists.txt:

```cmake
add_subdirectory(external/CppWindow)
target_link_libraries(your_project PRIVATE cppwindow::cppwindow)
```

For local development with multiple CMake build directories, the presets make
CMake download GLFW once into `.deps/glfw`. Each build directory still compiles
GLFW with its own flags.

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
Example targets include `example_imgui_minimal`, `example_imgui_overlay`,
`example_imgui_input_capture`, and `example_imgui_style_demo`.

## 🚀 Quick Start

### Window Creation

```cpp
#include <cppwindow/cppwindow.hpp>

int main()
{
    auto& ctx = cwin::WindowContext::get();

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
auto& ctx = cwin::WindowContext::get();

auto window =
    cwin::WindowBuilder{}
        .openGL({4,5,true})
        .build();

window.makeContextCurrent();
gladLoadGLLoader(reinterpret_cast<GLADloadproc>(ctx.getProcLoader()));
```

Render loop:

```cpp
while (!window.shouldClose())
{
    cwin::WindowContext::get().pollEvents();

    glClearColor(0.f,0.f,0.f,1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    window.swapBuffers();
}
```

### 🎮 Input

Input is tracked per window, avoiding hidden global state.

```cpp
const auto& input = window.getInput();

if (input.isKeyPressed(cwin::Key::Escape))
    window.requestClose();

auto [x,y] = input.getMousePosition();
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
bool decorated = window.isDecorated();
bool maximized = window.isMaximized();
```

See the [CppWindow Guide](docs/guide.md) for fullscreen mode semantics,
high-DPI notes, and monitor APIs. See the [API Reference](docs/api.md) for all
public types and methods.

## ⚖️ License

Licensed under the MIT License. See the [LICENSE](LICENSE) file for full text.
