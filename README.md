# CppWindow

CppWindow is a modern C++20 windowing and input abstraction built on top of GLFW, designed for high-performance graphics applications and engine development.

It provides a clean, type-safe API for window creation, event processing, and per-window input while keeping backend details fully hidden.

CppWindow is not a rendering library - it is a platform layer intended to sit beneath Vulkan, OpenGL, or future graphics backends.

## ✨ Features

* Modern C++20 design (RAII, spans, variants, concepts)
* Builder-based window creation
* Per-window input state
* Strongly typed event system
* Vulkan surface support
* OpenGL context support
* Zero global input state
* Backend abstraction (currently GLFW)
* Minimal runtime overhead

## 📦 Requirements

- **CMake**: 3.20 or higher
- **C++ Compiler**: Support for C++20 (e.g., GCC 11+, Clang 13+, MSVC 19.29+)
- **Dependencies**: GLFW 3.4 (Automatically managed via `FetchContent`)

## 🛠 Integration

### Git Submodule

#### Add the submodule to your project:

```bash
git submodule add [git@github.com:umrao-ak47/CppWindow.git](git@github.com:umrao-ak47/CppWindow.git) external/CppWindow
```

#### Include it in your CMakeLists.txt:

```cmake
add_subdirectory(external/CppWindow)
target_link_libraries(your_project PRIVATE cppwindow::cppwindow)
```

## 🚀 Quick Start

### Window Creation


```cpp
#include <cppwindow/window.hpp>

int main()
{
    cppwindow::Window window =
        cppwindow::WindowBuilder{}
            .title("Basic Example")
            .size(1280, 720)
            .noAPI()
            .build();

    auto& ctx = cppwindow::WindowContext::Get();

    while (!window.shouldClose())
    {
        ctx.pollEvents();

        for (const auto& event : window.events())
        {
            if (event.is<cppwindow::Event::Closed>())
                window.requestClose();
        }
    }
}
```

### 🧱 Graphics Modes

CppWindow supports two creation paths:

#### No API (Recommended for Vulkan)

```cpp
auto window =
    cppwindow::WindowBuilder{}
        .noAPI()
        .build();
```

Create the surface later:

```cpp
VkHandle surface =
    window.createVulkanSurface(instance);
```

#### OpenGL

CppWindow creates the context, but you must load OpenGL functions.

Example using GLAD:
```cpp
auto window =
    cppwindow::WindowBuilder{}
        .openGL({4,5,true})
        .build();

gladLoadGLLoader(window.getGLLoader());
```

Render loop:

```cpp
while (!window.shouldClose())
{
    cppwindow::WindowContext::Get().pollEvents();

    glClearColor(0.f,0.f,0.f,1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    window.swapBuffers();
}
```

### 🎮 Input

Input is tracked per window, avoiding hidden global state.

```cpp
const auto& input = window.getInput();

if (input.isKeyPressed(cppwindow::Key::Escape))
    window.requestClose();

auto [x,y] = input.getMousePosition();
```

### 📬 Events

Events are stored internally and exposed as a span to avoid allocations.

```cpp
for (const auto& event : window.events())
{
    event.visit([](auto&& e)
    {
        using T = std::decay_t<decltype(e)>;

        if constexpr (std::is_same_v<T, cppwindow::Event::Resized>)
        {
            // handle resize
        }
    });
}
```

## ⚖️ License

Licensed under the MIT License. See the [LICENSE](LICENSE) file for full text.
