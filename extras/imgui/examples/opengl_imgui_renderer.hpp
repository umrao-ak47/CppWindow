#ifndef CPPWINDOW_IMGUI_EXAMPLES_OPENGL_IMGUI_RENDERER_HPP
#define CPPWINDOW_IMGUI_EXAMPLES_OPENGL_IMGUI_RENDERER_HPP

#include <cppwindow/cppwindow.hpp>

#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <stdexcept>

namespace example {

inline void loadOpenGL(cwin::WindowContext& context)
{
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(context.getProcLoader()))) {
        throw std::runtime_error("Failed to initialize GLAD");
    }
}

class OpenGLImGuiRenderer final
{
public:
    explicit OpenGLImGuiRenderer(const char* glslVersion)
    {
        if (!ImGui_ImplOpenGL3_Init(glslVersion)) {
            throw std::runtime_error("Failed to initialize ImGui OpenGL renderer");
        }
    }

    ~OpenGLImGuiRenderer()
    {
        ImGui_ImplOpenGL3_Shutdown();
    }

    OpenGLImGuiRenderer(const OpenGLImGuiRenderer&) = delete;
    OpenGLImGuiRenderer& operator=(const OpenGLImGuiRenderer&) = delete;
    OpenGLImGuiRenderer(OpenGLImGuiRenderer&&) = delete;
    OpenGLImGuiRenderer& operator=(OpenGLImGuiRenderer&&) = delete;

    void newFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
    }

    void render(ImDrawData* drawData)
    {
        ImGui_ImplOpenGL3_RenderDrawData(drawData);
    }
};

inline void clearFramebuffer(cwin::Window& window, const ImVec4& color)
{
    auto [fbWidth, fbHeight] = window.getFramebufferSize();
    glViewport(0, 0, static_cast<GLsizei>(fbWidth), static_cast<GLsizei>(fbHeight));
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT);
}

inline void drawRect(int x, int y, int width, int height, const ImVec4& color)
{
    if (width <= 0 || height <= 0) {
        return;
    }

    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, width, height);
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
}

}  // namespace example

#endif  // CPPWINDOW_IMGUI_EXAMPLES_OPENGL_IMGUI_RENDERER_HPP
