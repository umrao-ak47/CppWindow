#include <cppwindow/cppwindow.hpp>
#include <cppwindow/imgui.hpp>

#include "opengl_imgui_renderer.hpp"

#include <imgui.h>

using namespace cwin;

int main()
{
    auto& ctx = WindowContext::get();

    auto window = WindowBuilder{}
                      .title("ImGui Style Demo")
                      .size(1100, 680)
                      .openGL({ 4, 1, true })
                      .resizable()
                      .build();
    window.makeContextCurrent();
    example::loadOpenGL(ctx);

    const bool useVSync = true;
    window.setVSync(useVSync);
    FrameLimiter frameLimiter(120.0);
    frameLimiter.setVSyncEnabled(useVSync);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    {
        cwin::imgui::Layer<example::OpenGLImGuiRenderer> imguiLayer(window, "#version 410");

        EventDispatcher dispatcher;
        dispatcher.on<Event::Closed>([&] {
            window.requestClose();
        });

        ImVec4 clearColor{ 0.06f, 0.07f, 0.08f, 1.0f };
        bool showDemo = true;
        bool showStyleEditor = true;

        while (!window.shouldClose()) {
            ctx.pollEvents();
            dispatcher.dispatch(window.events());
            imguiLayer.handleEvents(window.events());
            imguiLayer.newFrame();

            ImGuiStyle& style = ImGui::GetStyle();
            ImGui::Begin("Style Controls");
            ImGui::Checkbox("Show demo window", &showDemo);
            ImGui::Checkbox("Show style editor", &showStyleEditor);
            ImGui::ColorEdit3("Clear color", &clearColor.x);
            ImGui::SliderFloat("Window rounding", &style.WindowRounding, 0.0f, 18.0f);
            ImGui::SliderFloat("Frame rounding", &style.FrameRounding, 0.0f, 18.0f);
            ImGui::SliderFloat("Grab rounding", &style.GrabRounding, 0.0f, 18.0f);
            ImGui::SliderFloat("Alpha", &style.Alpha, 0.35f, 1.0f);
            ImGui::End();

            if (showStyleEditor) {
                ImGui::Begin("Style Editor", &showStyleEditor);
                ImGui::ShowStyleEditor();
                ImGui::End();
            }

            if (showDemo) {
                ImGui::ShowDemoWindow(&showDemo);
            }

            example::clearFramebuffer(window, clearColor);
            imguiLayer.render();
            window.swapBuffers();
            frameLimiter.wait();
        }
    }

    ImGui::DestroyContext();
}
