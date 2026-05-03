#include <cppwindow/cppwindow.hpp>

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
    std::cout << "Skipping runtime smoke test: " << error.what() << '\n';
    return SkipTest;
}

}  // namespace

int main()
{
#if defined(__APPLE__)
    std::cout << "Skipping runtime smoke test on macOS; hidden GLFW teardown can hang in "
                 "non-interactive runners\n";
    return SkipTest;
#else
    try {
        auto& context = cwin::WindowContext::get();

        auto utilityWindow = cwin::WindowBuilder{}
                                 .title("CppWindow Runtime Smoke")
                                 .size(320, 240)
                                 .hidden()
                                 .noGraphicsApi()
                                 .build();

        if (utilityWindow.isVisible()) {
            std::cerr << "Hidden utility window was reported visible\n";
            return 1;
        }

        const auto [width, height] = utilityWindow.getSize();
        if (width <= 0 || height <= 0) {
            std::cerr << "Utility window reported invalid size\n";
            return 1;
        }

        if (utilityWindow.shouldClose()) {
            std::cerr << "New utility window unexpectedly requested close\n";
            return 1;
        }
        utilityWindow.requestClose();
        if (!utilityWindow.shouldClose()) {
            std::cerr << "requestClose did not mark utility window for close\n";
            return 1;
        }

        auto glWindow = cwin::WindowBuilder{}
                            .title("CppWindow OpenGL Smoke")
                            .size(64, 64)
                            .hidden()
                            .openGL({ 4, 1, true })
                            .build();
        glWindow.makeContextCurrent();

        const auto loader = context.getProcLoader();
        if (loader == nullptr || loader("glGetString") == nullptr) {
            std::cerr << "OpenGL procedure loader did not resolve glGetString\n";
            return 1;
        }

        glWindow.setVSync(false);
        context.pollEvents();
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
#endif
}
