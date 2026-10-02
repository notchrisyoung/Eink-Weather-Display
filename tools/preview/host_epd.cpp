// Minimal PC stand-ins for the parts of the EPD47 driver the screen code uses,
// so the real layout code can be rendered to a PNG without hardware.
#include "epd_driver.h"
#include <zlib.h>
#include <vector>
#include <cstring>

extern "C" {

void epd_draw_pixel(int32_t x, int32_t y, uint8_t color, uint8_t *fb) {
    if (x < 0 || x >= EPD_WIDTH || y < 0 || y >= EPD_HEIGHT) return;
    uint8_t &b = fb[y * EPD_WIDTH / 2 + x / 2];
    b = (x & 1) ? (uint8_t)((b & 0x0F) | (color & 0xF0)) : (uint8_t)((b & 0xF0) | (color >> 4));
}

static uint32_t nextCodepoint(const char *&s) {
    const unsigned char *p = (const unsigned char *)s;
    uint32_t cp;
    int extra;
    if (p[0] < 0x80) { cp = p[0]; extra = 0; }
    else if ((p[0] & 0xE0) == 0xC0) { cp = p[0] & 0x1F; extra = 1; }
    else if ((p[0] & 0xF0) == 0xE0) { cp = p[0] & 0x0F; extra = 2; }
    else { cp = p[0] & 0x07; extra = 3; }
    for (int i = 1; i <= extra; i++) cp = (cp << 6) | (p[i] & 0x3F);
    s += 1 + extra;
    return cp;
}

void get_glyph(const GFXfont *font, uint32_t cp, GFXglyph **glyph) {
    *glyph = nullptr;
    for (uint32_t i = 0; i < font->interval_count; i++) {
        const UnicodeInterval &r = font->intervals[i];
        if (cp >= r.first && cp <= r.last) { *glyph = &font->glyph[r.offset + cp - r.first]; return; }
    }
}

void get_text_bounds(const GFXfont *font, const char *s, int32_t *x, int32_t *y,
                     int32_t *x1, int32_t *y1, int32_t *w, int32_t *h, const FontProperties *) {
    int32_t minx = 1 << 20, miny = 1 << 20, maxx = -(1 << 20), maxy = -(1 << 20), x0 = *x;
    while (*s) {
        GFXglyph *g;
        get_glyph(font, nextCodepoint(s), &g);
        if (!g) continue;
        int32_t gx = *x + g->left, gy = *y - g->top;
        if (gx < minx) minx = gx;
        if (gy < miny) miny = gy;
        if (gx + g->width > maxx) maxx = gx + g->width;
        if (gy + g->height > maxy) maxy = gy + g->height;
        *x += g->advance_x;
    }
    *x1 = minx < x0 ? minx : x0; *y1 = miny; *w = maxx - *x1; *h = maxy - miny;
}

void write_mode(const GFXfont *font, const char *s, int32_t *cx, int32_t *cy, uint8_t *fb,
                DrawMode_t, const FontProperties *props) {
    uint8_t fg = props ? props->fg_color : 0, bg = props ? props->bg_color : 15;
    while (*s) {
        GFXglyph *g;
        uint32_t cp = nextCodepoint(s);
        get_glyph(font, cp, &g);
        if (!g && props) get_glyph(font, props->fallback_glyph, &g);
        if (!g) continue;
        int bw = (g->width + 1) / 2;
        std::vector<uint8_t> bmp(bw * g->height + 1);
        uLongf len = bmp.size();
        uncompress(bmp.data(), &len, font->bitmap + g->data_offset, g->compressed_size);
        for (int yy = 0; yy < g->height; yy++)
            for (int xx = 0; xx < g->width; xx++) {
                uint8_t v = bmp[yy * bw + xx / 2];
                v = (xx & 1) ? (v >> 4) : (v & 0x0F);
                int tone = bg + (int)v * ((int)fg - (int)bg) / 15;
                epd_draw_pixel(*cx + g->left + xx, *cy - g->top + yy, (uint8_t)(tone * 0x11), fb);
            }
        *cx += g->advance_x;
    }
}

}  // extern "C"
