#include <cppwindow/cppwindow.hpp>

#include <iostream>
#include <string>

using namespace cwin;

namespace {

void appendUtf8(std::string& out, char32_t codepoint)
{
    if (codepoint <= 0x7F) {
        out.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else if (codepoint <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
}
std::string toUtf8(const std::u32string& text)
{
    std::string result;
    for (char32_t codepoint : text) {
        appendUtf8(result, codepoint);
    }
    return result;
}

}  // namespace

int main()
{
    auto& ctx = WindowContext::get();

    auto window = WindowBuilder{}
                      .title("Text Input")
                      .size(720, 360)
                      .noGraphicsApi()
                      .resizable()
                      .build();

    std::cout << "Controls:\n"
              << "  Type text: append to title\n"
              << "  Backspace: delete last character\n"
              << "  Esc: close\n";

    std::u32string text;
    FrameLimiter frameLimiter(60.0);
    EventDispatcher dispatcher;
    dispatcher
        .on<Event::Closed>([&] {
            window.requestClose();
        })
        .on<Event::KeyPressed>([&](const Event::KeyPressed& key) {
            if (key.key == Key::Escape) {
                window.requestClose();
            } else if (key.key == Key::Backspace && !text.empty()) {
                text.pop_back();
            }
        })
        .on<Event::TextEntered>([&](const Event::TextEntered& entered) {
            if (entered.unicode >= U' ' && entered.unicode != 0x7F) {
                text.push_back(entered.unicode);
            }
        });

    while (!window.shouldClose()) {
        ctx.pollEvents();
        dispatcher.dispatch(window.events());

        std::string title = "Text Input";
        if (!text.empty()) {
            title += " - " + toUtf8(text);
        }
        window.setTitle(title);

        frameLimiter.wait();
    }
}
