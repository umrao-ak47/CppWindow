#include <cppwindow/cppwindow.hpp>
#include <cppwindow/imgui.hpp>

#include "opengl_imgui_renderer.hpp"

#include <imgui.h>

using namespace cwin;

int main()
{
    auto& ctx = WindowContext::get();

    auto window = WindowBuilder{}
                      .title("ImGui Minimal")
                      .size(900, 520)
                      .openGL({ 4, 1, true })
                      .resizable()
                      .build();
    window.makeContextCurrent();
    example::loadOpenGL(ctx);

    const bool useVSync = true;
    window.setVSync(useVSync);
    FrameLimiter frameLimiter(120.0);
    frameLimiter.setVSyncEnabled(useVSync);

    cwin::imgui::Context imguiContext{ { .style = cwin::imgui::Style::Dark } };

    {
        cwin::imgui::Layer<example::OpenGLImGuiRenderer> imguiLayer(window, "#version 410");

        EventDispatcher dispatcher;
        dispatcher.on<Event::Closed>([&] {
            window.requestClose();
        });

        FrameTimer frameTimer;
        FpsCounter fpsCounter;
        ImVec4 clearColor{ 0.09f, 0.11f, 0.13f, 1.0f };
        bool showDemo = false;

        while (!window.shouldClose()) {
            ctx.pollEvents();
            dispatcher.dispatch(window.events());
            imguiLayer.handleEvents(window.events());
            imguiLayer.newFrame();

            const FrameTime frameTime = frameTimer.tick();
            (void)fpsCounter.update(frameTime);

            ImGui::Begin("Minimal");
            ImGui::Text("FPS %.1f", fpsCounter.framesPerSecond());
            ImGui::ColorEdit3("Clear color", &clearColor.x);
            ImGui::Checkbox("Show Dear ImGui demo", &showDemo);
            ImGui::End();

            if (showDemo) {
                ImGui::ShowDemoWindow(&showDemo);
            }

            example::clearFramebuffer(window, clearColor);
            imguiLayer.render();
            window.swapBuffers();
            frameLimiter.wait();
        }
    }

}
