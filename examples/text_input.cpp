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
    auto& ctx = WindowContext::Get();

    auto window = WindowBuilder{}.title("Text Input").size(720, 360).noAPI().resizable().build();

    std::cout << "Type text. Backspace deletes, Escape closes.\n";

    std::u32string text;
    FrameLimiter frameLimiter(60.0);

    while (!window.shouldClose()) {
        ctx.pollEvents();

        for (const auto& event : window.events()) {
            if (event.is<Event::Closed>()) {
                window.requestClose();
            }

            if (const auto* key = event.getIf<Event::KeyPressed>()) {
                if (key->key == Key::Escape) {
                    window.requestClose();
                } else if (key->key == Key::Backspace && !text.empty()) {
                    text.pop_back();
                }
            }

            if (const auto* entered = event.getIf<Event::TextEntered>()) {
                if (entered->unicode >= U' ' && entered->unicode != 0x7F) {
                    text.push_back(entered->unicode);
                }
            }
        }

        std::string title = "Text Input";
        if (!text.empty()) {
            title += " - " + toUtf8(text);
        }
        window.setTitle(title);

        frameLimiter.wait();
    }
}
