#include <cppwindow/cppwindow.hpp>
#include <cppwindow/imgui.hpp>

#include <algorithm>
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <stdexcept>

using namespace cwin;

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

int main()
{
    auto& ctx = WindowContext::get();

    auto window = WindowBuilder{}
                      .title("ImGui Overlay")
                      .size(960, 540)
                      .openGL({ 4, 1, true })
                      .resizable()
                      .build();
    window.makeContextCurrent();

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(ctx.getProcLoader()))) {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    const bool useVSync = true;
    window.setVSync(useVSync);
    FrameLimiter frameLimiter(120.0);
    frameLimiter.setVSyncEnabled(useVSync);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    {
        cwin::imgui::Layer<OpenGLImGuiRenderer> imguiLayer(window, "#version 410");

        ActionMap actions;
        const ActionId quit = actions.getOrCreateActionId("quit");
        const ActionId moveLeft = actions.getOrCreateActionId("move_left");
        const ActionId moveRight = actions.getOrCreateActionId("move_right");
        actions.bindKey(quit, Key::Escape)
            .bindKey(moveLeft, Key::A)
            .bindKey(moveRight, Key::D)
            .setContext(moveLeft, "gameplay")
            .setContext(moveRight, "gameplay");

        EventDispatcher dispatcher;
        dispatcher.on<Event::Closed>([&] {
            window.requestClose();
        });

        FrameTimer frameTimer;
        FpsCounter fpsCounter;
        float markerPosition = 0.5f;
        bool showDemo = false;
        ImVec4 clearColor{ 0.045f, 0.055f, 0.065f, 1.0f };

        while (!window.shouldClose()) {
            ctx.pollEvents();
            dispatcher.dispatch(window.events());
            imguiLayer.handleEvents(window.events());
            imguiLayer.newFrame();

            const FrameTime frameTime = frameTimer.tick();
            (void)fpsCounter.update(frameTime);

            const bool uiCapturesInput = imguiLayer.wantsMouse() || imguiLayer.wantsKeyboard();
            actions.setContextEnabled("gameplay", !uiCapturesInput);
            actions.update(window.getInput());

            if (actions.isPressed(quit)) {
                window.requestClose();
            }

            float movement = 0.0f;
            if (actions.isDown(moveLeft)) {
                movement -= 1.0f;
            }
            if (actions.isDown(moveRight)) {
                movement += 1.0f;
            }
            markerPosition = std::clamp(
                markerPosition + movement * static_cast<float>(frameTime.deltaSeconds) * 0.5f,
                0.0f,
                1.0f);

            ImGui::Begin("CppWindow ImGui");
            ImGui::Text("FPS %.1f", fpsCounter.framesPerSecond());
            ImGui::Text("Mouse capture: %s", imguiLayer.wantsMouse() ? "yes" : "no");
            ImGui::Text("Keyboard capture: %s", imguiLayer.wantsKeyboard() ? "yes" : "no");
            ImGui::Checkbox("Show ImGui demo", &showDemo);
            ImGui::ColorEdit3("Clear color", &clearColor.x);
            ImGui::ProgressBar(markerPosition, ImVec2(-1.0f, 0.0f), "A/D gameplay action");
            ImGui::End();

            if (showDemo) {
                ImGui::ShowDemoWindow(&showDemo);
            }

            auto [fbWidth, fbHeight] = window.getFramebufferSize();
            glViewport(0, 0, static_cast<GLsizei>(fbWidth), static_cast<GLsizei>(fbHeight));
            glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
            glClear(GL_COLOR_BUFFER_BIT);

            imguiLayer.render();
            window.swapBuffers();

            frameLimiter.wait();
        }
    }

    ImGui::DestroyContext();
}
