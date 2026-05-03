#ifndef CPPWINDOW_IMGUI_EXAMPLES_OPENGL_IMGUI_RENDERER_HPP
#define CPPWINDOW_IMGUI_EXAMPLES_OPENGL_IMGUI_RENDERER_HPP

#include <cppwindow/cppwindow.hpp>

#include <imgui.h>

#include <memory>

namespace example {

void loadOpenGL(cwin::WindowContext& context);

class OpenGLImGuiRenderer final
{
public:
    explicit OpenGLImGuiRenderer(const char* glslVersion);
    ~OpenGLImGuiRenderer();

    OpenGLImGuiRenderer(const OpenGLImGuiRenderer&) = delete;
    OpenGLImGuiRenderer& operator=(const OpenGLImGuiRenderer&) = delete;
    OpenGLImGuiRenderer(OpenGLImGuiRenderer&&) = delete;
    OpenGLImGuiRenderer& operator=(OpenGLImGuiRenderer&&) = delete;

    void newFrame();
    void render(ImDrawData* drawData);

private:
    struct State;
    std::unique_ptr<State> state_;
};

void clearFramebuffer(cwin::Window& window, const ImVec4& color);
void drawRect(int x, int y, int width, int height, const ImVec4& color);

}  // namespace example

#endif  // CPPWINDOW_IMGUI_EXAMPLES_OPENGL_IMGUI_RENDERER_HPP
