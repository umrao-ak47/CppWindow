#ifndef CPPWINDOW_EXAMPLES_STATUS_TEXT_HPP
#define CPPWINDOW_EXAMPLES_STATUS_TEXT_HPP

#include <glad/glad.h>

#include <array>
#include <cstdint>
#include <string>

namespace example {

using Glyph = std::array<const char*, 7>;

inline Glyph glyphFor(char c)
{
    switch (c) {
        case 'A':
            return { "01110", "10001", "10001", "11111", "10001", "10001", "10001" };
        case 'C':
            return { "11110", "10001", "10000", "10000", "10000", "10001", "11110" };
        case 'D':
            return { "11110", "10001", "10001", "10001", "10001", "10001", "11110" };
        case 'E':
            return { "11111", "10000", "10000", "11110", "10000", "10000", "11111" };
        case 'F':
            return { "11111", "10000", "10000", "11110", "10000", "10000", "10000" };
        case 'I':
            return { "11111", "00100", "00100", "00100", "00100", "00100", "11111" };
        case 'N':
            return { "10001", "11001", "10101", "10011", "10001", "10001", "10001" };
        case 'O':
            return { "01110", "10001", "10001", "10001", "10001", "10001", "01110" };
        case 'P':
            return { "11110", "10001", "10001", "11110", "10000", "10000", "10000" };
        case 'R':
            return { "11110", "10001", "10001", "11110", "10100", "10010", "10001" };
        case 'S':
            return { "01111", "10000", "10000", "01110", "00001", "00001", "11110" };
        case 'T':
            return { "11111", "00100", "00100", "00100", "00100", "00100", "00100" };
        case 'U':
            return { "10001", "10001", "10001", "10001", "10001", "10001", "01110" };
        case 'X':
            return { "10001", "10001", "01010", "00100", "01010", "10001", "10001" };
        case 'Y':
            return { "10001", "10001", "01010", "00100", "00100", "00100", "00100" };
        case '0':
            return { "01110", "10001", "10011", "10101", "11001", "10001", "01110" };
        case '1':
            return { "00100", "01100", "00100", "00100", "00100", "00100", "01110" };
        case '2':
            return { "01110", "10001", "00001", "00010", "00100", "01000", "11111" };
        case '3':
            return { "11110", "00001", "00001", "01110", "00001", "00001", "11110" };
        case '4':
            return { "00010", "00110", "01010", "10010", "11111", "00010", "00010" };
        case '5':
            return { "11111", "10000", "10000", "11110", "00001", "00001", "11110" };
        case '6':
            return { "01110", "10000", "10000", "11110", "10001", "10001", "01110" };
        case '7':
            return { "11111", "00001", "00010", "00100", "01000", "01000", "01000" };
        case '8':
            return { "01110", "10001", "10001", "01110", "10001", "10001", "01110" };
        case '9':
            return { "01110", "10001", "10001", "01111", "00001", "00001", "01110" };
        case '+':
            return { "00000", "00100", "00100", "11111", "00100", "00100", "00000" };
        case '-':
            return { "00000", "00000", "00000", "11111", "00000", "00000", "00000" };
        case '.':
            return { "00000", "00000", "00000", "00000", "00000", "01100", "01100" };
        case ':':
            return { "00000", "01100", "01100", "00000", "01100", "01100", "00000" };
        default:
            return { "00000", "00000", "00000", "00000", "00000", "00000", "00000" };
    }
}

inline void drawRect(int x, int y, int width, int height, float red, float green, float blue)
{
    if (width <= 0 || height <= 0) {
        return;
    }

    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, width, height);
    glClearColor(red, green, blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
}

inline void drawText(const std::string& text, int x, int y, int scale)
{
    constexpr int GlyphWidth = 5;
    constexpr int GlyphHeight = 7;

    int cursorX = x;
    for (char c : text) {
        if (c == ' ') {
            cursorX += 4 * scale;
            continue;
        }

        const Glyph glyph = glyphFor(c);
        for (int row = 0; row < GlyphHeight; ++row) {
            for (int col = 0; col < GlyphWidth; ++col) {
                if (glyph[row][col] == '1') {
                    const int pixelX = cursorX + col * scale;
                    const int pixelY = y + (GlyphHeight - 1 - row) * scale;
                    drawRect(pixelX, pixelY, scale, scale, 0.88f, 0.94f, 0.96f);
                }
            }
        }

        cursorX += (GlyphWidth + 1) * scale;
    }
}

inline void drawStatusBar(uint32_t framebufferWidth, uint32_t framebufferHeight, const std::string& text)
{
    glViewport(
        0,
        0,
        static_cast<GLsizei>(framebufferWidth),
        static_cast<GLsizei>(framebufferHeight));
    glClearColor(0.045f, 0.055f, 0.065f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    const int width = static_cast<int>(framebufferWidth);
    const int barHeight = framebufferWidth < 900 ? 30 : 42;
    const int scale = framebufferWidth < 900 ? 1 : 2;
    drawRect(0, 0, width, barHeight, 0.02f, 0.025f, 0.03f);
    drawRect(0, barHeight - 2, width, 2, 0.14f, 0.45f, 0.70f);
    drawText(text, 12, (barHeight - 7 * scale) / 2, scale);
}

}  // namespace example

#endif
