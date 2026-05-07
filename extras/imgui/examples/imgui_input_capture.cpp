#include <cppwindow/cppwindow.hpp>
#include <cppwindow/imgui.hpp>

#include "opengl_imgui_renderer.hpp"

#include <algorithm>
#include <array>
#include <imgui.h>

using namespace cwin;

int main()
{
    auto& ctx = WindowContext::get();

    auto window = WindowBuilder{}
                      .title("ImGui Input Capture")
                      .size(960, 540)
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

        ActionMap actions;
        const ActionId quit = actions.defineAction("quit");
        const ActionId moveLeft = actions.defineAction("move_left");
        const ActionId moveRight = actions.defineAction("move_right");
        const ActionId moveUp = actions.defineAction("move_up");
        const ActionId moveDown = actions.defineAction("move_down");
        const ActionId boost = actions.defineAction("boost");
        actions.bindKey(quit, Key::Escape)
            .bindKey(moveLeft, Key::A)
            .bindKey(moveRight, Key::D)
            .bindKey(moveUp, Key::W)
            .bindKey(moveDown, Key::S)
            .bindKey(boost, Key::Space)
            .setContext(moveLeft, "gameplay")
            .setContext(moveRight, "gameplay")
            .setContext(moveUp, "gameplay")
            .setContext(moveDown, "gameplay")
            .setContext(boost, "gameplay");

        FrameTimer frameTimer;
        float markerX = 0.5f;
        float markerY = 0.5f;
        std::array<char, 96> textBuffer{ "Click here, then A/D/W/S should not move the square" };

        while (!window.shouldClose()) {
            ctx.pollEvents();
            dispatcher.dispatch(window.events());
            imguiLayer.handleEvents(window.events());
            imguiLayer.newFrame();

            ImGui::Begin("Input Capture");
            ImGui::Text("A/D/W/S move the square");
            ImGui::Text("Space boosts movement");
            ImGui::InputText("Text input", textBuffer.data(), textBuffer.size());
            ImGui::Text("Mouse capture: %s", imguiLayer.wantsMouse() ? "yes" : "no");
            ImGui::Text("Keyboard capture: %s", imguiLayer.wantsKeyboard() ? "yes" : "no");
            ImGui::End();

            const bool uiCapturesInput = imguiLayer.wantsMouse() || imguiLayer.wantsKeyboard();
            actions.setContextEnabled("gameplay", !uiCapturesInput);
            actions.update(window.input());

            if (actions.isPressed(quit)) {
                window.requestClose();
            }

            const FrameTime frameTime = frameTimer.tick();
            const float speed =
                actions.isDown(boost) ? 0.75f : 0.35f;
            float xMovement = 0.0f;
            float yMovement = 0.0f;
            if (actions.isDown(moveLeft)) {
                xMovement -= 1.0f;
            }
            if (actions.isDown(moveRight)) {
                xMovement += 1.0f;
            }
            if (actions.isDown(moveDown)) {
                yMovement -= 1.0f;
            }
            if (actions.isDown(moveUp)) {
                yMovement += 1.0f;
            }

            markerX = std::clamp(
                markerX + xMovement * speed * static_cast<float>(frameTime.deltaSeconds),
                0.0f,
                1.0f);
            markerY = std::clamp(
                markerY + yMovement * speed * static_cast<float>(frameTime.deltaSeconds),
                0.0f,
                1.0f);

            example::clearFramebuffer(window, { 0.045f, 0.055f, 0.065f, 1.0f });
            auto [fbWidth, fbHeight] = window.framebufferSize();
            constexpr int MarkerSize = 54;
            const int markerPixelX = static_cast<int>(
                markerX * static_cast<float>(fbWidth > MarkerSize ? fbWidth - MarkerSize : 0));
            const int markerPixelY = static_cast<int>(
                markerY * static_cast<float>(fbHeight > MarkerSize ? fbHeight - MarkerSize : 0));
            example::drawRect(
                markerPixelX,
                markerPixelY,
                MarkerSize,
                MarkerSize,
                uiCapturesInput ? ImVec4{ 0.85f, 0.36f, 0.28f, 1.0f }
                                : ImVec4{ 0.25f, 0.72f, 0.58f, 1.0f });

            imguiLayer.render();
            window.swapBuffers();
            frameLimiter.wait();
        }
    }

}
