# Third-Party Notices

CppWindow is licensed under the MIT License. This file records third-party
software used, fetched, or bundled by this repository.

## GLFW

- Role: default windowing backend, fetched with CMake `FetchContent` when no
  existing `glfw` target is provided.
- Version used by default: 3.4
- Project: https://www.glfw.org/
- Source: https://github.com/glfw/glfw
- License: zlib/libpng
- Upstream license file:
  https://github.com/glfw/glfw/blob/master/LICENSE.md

## Dear ImGui

- Role: optional ImGui integration dependency when `CPPWINDOW_BUILD_IMGUI=ON`
  and no user-provided Dear ImGui target is supplied.
- Version used by default: v1.92.7
- Source: https://github.com/ocornut/imgui
- License: MIT
- Upstream license file:
  https://github.com/ocornut/imgui/blob/master/LICENSE.txt

## glad

- Role: generated OpenGL loader used only by examples. The target is private to
  examples and is not exported by CppWindow's install/package configuration.
- Generator: glad 0.1.36
- Generated API: OpenGL 4.6 compatibility profile
- Source: https://github.com/Dav1dde/glad
- License notes:
  https://github.com/Dav1dde/glad#license

## Khronos khrplatform.h

- Role: bundled Khronos platform header included by the example glad loader.
- Source: https://github.com/KhronosGroup/EGL-Registry
- License: MIT-style Khronos license
- Local license text:
  [examples/glad/include/KHR/khrplatform.h](examples/glad/include/KHR/khrplatform.h)
