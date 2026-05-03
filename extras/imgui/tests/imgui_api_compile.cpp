#include <cppwindow/cppwindow.hpp>
#include <cppwindow/imgui.hpp>

#include <span>
#include <type_traits>
#include <utility>

struct TestRenderer {
    void newFrame();
    void render(ImDrawData*);
};

static_assert(cwin::imgui::Renderer<TestRenderer>);

static_assert(std::is_same_v<
              decltype(std::declval<cwin::imgui::Platform&>().handleEvents(
                  std::declval<std::span<const cwin::Event>>())),
              void>);
static_assert(
    std::is_same_v<decltype(std::declval<cwin::imgui::Platform&>().newFrame()), void>);
static_assert(
    std::is_same_v<decltype(std::declval<const cwin::imgui::Platform&>().wantsMouse()), bool>);
static_assert(
    std::is_same_v<decltype(std::declval<const cwin::imgui::Platform&>().wantsKeyboard()), bool>);
static_assert(
    std::is_same_v<decltype(std::declval<const cwin::imgui::Platform&>().wantsTextInput()), bool>);
static_assert(std::is_same_v<
              decltype(std::declval<cwin::imgui::Platform&>().setMouseCursorUpdatesEnabled(true)),
              void>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::imgui::Platform&>()
                           .mouseCursorUpdatesEnabled()),
              bool>);
static_assert(
    std::is_same_v<decltype(std::declval<cwin::imgui::Layer<TestRenderer>&>().handleEvents(
                       std::declval<std::span<const cwin::Event>>())),
                   void>);
static_assert(
    std::is_same_v<decltype(std::declval<cwin::imgui::Layer<TestRenderer>&>().newFrame()), void>);
static_assert(std::is_same_v<
              decltype(std::declval<cwin::imgui::Layer<TestRenderer>&>().render()),
              void>);
static_assert(std::is_same_v<
              decltype(std::declval<cwin::imgui::Layer<TestRenderer>&>().renderer()),
              TestRenderer&>);
static_assert(std::is_same_v<
              decltype(std::declval<cwin::imgui::Layer<TestRenderer>&>().platform()),
              cwin::imgui::Platform&>);

int main()
{
    return 0;
}
