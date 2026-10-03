#pragma once
// Thin drawing layer over the EPD47 framebuffer: 16 grey levels, aligned
// text, thick lines and a few shapes the screen layout needs.
#include <stdint.h>
#include "epd_driver.h"

// Grey levels, 0 = black ink ... 15 = bare paper.
namespace Tone {
    constexpr uint8_t Ink   = 0;
    constexpr uint8_t Dark  = 4;
    constexpr uint8_t Mid   = 8;
    constexpr uint8_t Soft  = 11;
    constexpr uint8_t Faint = 13;
    constexpr uint8_t Whisper = 14;   // lightest grey the panel can show
    constexpr uint8_t Paper = 15;
}

enum class Align { Left, Center, Right };

class Canvas {
public:
    static const int W = EPD_WIDTH;
    static const int H = EPD_HEIGHT;

    explicit Canvas(uint8_t *framebuffer) : fb_(framebuffer) {}
    uint8_t *buffer() const { return fb_; }

    void fill(uint8_t tone);
    void pixel(int x, int y, uint8_t tone);
    void hline(int x, int y, int len, uint8_t tone);
    void vline(int x, int y, int len, uint8_t tone);
    void line(int x0, int y0, int x1, int y1, uint8_t tone, int thickness = 1);
    // Smooth (anti-aliased) line with sub-pixel end points and round caps.
    void stroke(float x0, float y0, float x1, float y1, float width, uint8_t tone);
    uint8_t get(int x, int y) const;
    void rect(int x, int y, int w, int h, uint8_t tone);
    void fillRect(int x, int y, int w, int h, uint8_t tone);
    void roundRect(int x, int y, int w, int h, int r, uint8_t tone);
    void fillRoundRect(int x, int y, int w, int h, int r, uint8_t tone);
    void circle(int cx, int cy, int r, uint8_t tone);
    void fillCircle(int cx, int cy, int r, uint8_t tone);
    void ring(int cx, int cy, int r, int thickness, uint8_t tone);
    void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint8_t tone);
    void dottedHline(int x, int y, int len, int gap, uint8_t tone);

    // Text is positioned by its baseline. Returns the x just past the text.
    // `paper` must match whatever is underneath, since glyph boxes are opaque.
    int text(const GFXfont &font, int x, int baseline, const char *s,
             Align align = Align::Left, uint8_t ink = Tone::Ink, uint8_t paper = Tone::Paper);
    int textWidth(const GFXfont &font, const char *s);

    // Smooth outlined shape described by a signed-distance function: sdf(x, y)
    // is negative inside, positive outside, in pixels. The outline sits just
    // inside the edge, `edge` pixels wide; both edges are anti-aliased.
    template <class SDF>
    void shape(int x0, int y0, int x1, int y1, SDF sdf, uint8_t fill, uint8_t outline, float edge) {
        for (int y = y0; y <= y1; y++)
            for (int x = x0; x <= x1; x++) {
                float d = sdf((float)x, (float)y);
                float outer = 0.5f - d;                 // coverage of the whole shape
                if (outer <= 0) continue;
                if (outer > 1) outer = 1;
                float inner = 0.5f - (d + edge);        // coverage of the fill inside the outline
                inner = inner < 0 ? 0 : (inner > 1 ? 1 : inner);
                float under = get(x, y);
                float v = under * (1 - outer) + outline * (outer - inner) + fill * inner;
                pixel(x, y, (uint8_t)(v + 0.5f));
            }
    }

private:
    uint8_t *fb_;
};
