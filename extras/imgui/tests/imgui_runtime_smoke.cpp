#include <cppwindow/cppwindow.hpp>
#include <cppwindow/imgui.hpp>

#include "opengl_imgui_renderer.hpp"

#include <imgui.h>

#include <iostream>

namespace {

constexpr int SkipTest = 77;

[[nodiscard]] bool isEnvironmentUnavailable(const cwin::Error& error) noexcept
{
    return error.code() == cwin::ErrorCode::InitializationFailed
        || error.code() == cwin::ErrorCode::WindowCreationFailed;
}

int skip(const std::exception& error)
{
    std::cout << "Skipping ImGui runtime smoke test: " << error.what() << '\n';
    return SkipTest;
}

}  // namespace

int main()
{
    try {
        auto& context = cwin::WindowContext::get();
        auto window = cwin::WindowBuilder{}
                          .title("CppWindow ImGui Runtime Smoke")
                          .size(320, 240)
                          .hidden()
                          .openGL({ 4, 1, true })
                          .build();
        window.makeContextCurrent();
        example::loadOpenGL(context);
        window.setVSync(false);

        cwin::imgui::Context imguiContext{ { .style = cwin::imgui::Style::Dark } };

        {
            cwin::imgui::Layer<example::OpenGLImGuiRenderer> imguiLayer(window, "#version 410");

            context.pollEvents();
            imguiLayer.handleEvents(window.events());
            imguiLayer.newFrame();

            ImGui::Begin("Runtime Smoke");
            ImGui::Text("CppWindow ImGui runtime path");
            ImGui::End();

            example::clearFramebuffer(window, { 0.02f, 0.03f, 0.04f, 1.0f });
            imguiLayer.render();
            context.pollEvents();
        }
    } catch (const cwin::Error& error) {
        if (isEnvironmentUnavailable(error)) {
            return skip(error);
        }
        std::cerr << error.what() << '\n';
        return 1;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }

    return 0;
}
