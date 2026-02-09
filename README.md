# glfw-vk

A modern C++20 wrapper for GLFW designed to provide a clean, object-oriented interface while isolating windowing logic from your rendering engine.

## 🚀 Goal

`glfw-vk` encapsulates GLFW entirely within its implementation:

- **Zero Header Leakage**: GLFW headers are used only in the `.cpp` files. Your public API remains pure C++.
- **Vulkan-Ready, Not Vulkan-Dependent**: Provides the necessary hooks to create Vulkan surfaces without requiring the Vulkan SDK to build this library.
- **RAII First**: Windows, contexts, and input states are managed via standard C++ lifecycles.

## ✨ Features

- **Modern C++20 API**: Leverages modern C++20 features.
- **Context Isolation**: Manage GLFW initialization and termination safely via the `Context` class.
- **Input Management**: Dedicated `InputState` and `InputMap` for clean event handling.
- **High Portability**: Designed to be used in Vulkan projects as a submodule.

## 📦 Requirements

- **CMake**: 3.20 or higher
- **C++ Compiler**: Support for C++20 (e.g., GCC 11+, Clang 13+, MSVC 19.29+)
- **Dependencies**: GLFW 3.4 (Automatically managed via `FetchContent`)

## 🛠 Integration

### Git Submodule

#### Add the submodule to your project:

```bash
git submodule add [https://github.com/umrao-ak47/glfw-vk.git](https://github.com/umrao-ak47/glfw-vk.git) external/glfw-vk
```

#### Include it in your CMakeLists.txt:

```cmake
add_subdirectory(external/glfw-vk)
target_link_libraries(your_project PRIVATE glfw-vk::glfw-vk)
```

## 📖 Usage

### Window Creation

```cpp
#include <glfw-vk/GLFWvk.hpp>

int main()
{
    // Manages GLFW init/destroy
    auto& context = glfwvk::GlfwContext::Get();
    glfwvk::WindowDesc windowDesc{
        .title = "My App",
        .width = 1280,
        .height = 720,
    };

    // Create a managed window
    glfwvk::Window window(windowDesc);

    while (!window.shouldClose()) {
        window.pollEvents();
        // Your rendering here
    }
}
```

### Vulkan Surface Creation

You can create a vulkan surface by passing your instance pointer:

```cpp
// Returns the raw VkSurfaceKHR handle as a uint64_t
Window::SurfaceHandle surface = window.createSurface(rawVkInstance);
```

## 📂 Project Structure

- include/: Public C++ headers (No GLFW/Vulkan includes).
- src/: Private implementation (Where GLFW lives).
- src/Context.cpp: Manages the global GLFW lifecycle.
- src/InputMap.cpp: Translates GLFW events into engine-friendly states.

## ⚖️ License

Licensed under the MIT License. See the [LICENSE](LICENSE) file for full text.
