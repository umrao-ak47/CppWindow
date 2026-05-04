#include <cppwindow/cppwindow.hpp>
#include <cppwindow/imgui.hpp>

#include <type_traits>
#include <utility>

struct TargetProviderRenderer {
    void newFrame();
    void render(ImDrawData*);
};

static_assert(cwin::imgui::Renderer<TargetProviderRenderer>);
static_assert(std::is_constructible_v<cwin::imgui::Context>);
static_assert(
    std::is_same_v<decltype(std::declval<cwin::imgui::Platform&>().newFrame()), void>);
static_assert(std::is_same_v<
              decltype(std::declval<cwin::imgui::Layer<TargetProviderRenderer>&>().render()),
              void>);

int main()
{
    return 0;
}
