#include "canvas.h"
#include <string.h>
#include <math.h>

static inline uint8_t toByte(uint8_t tone) { return (uint8_t)((tone & 0x0F) * 0x11); }

void Canvas::fill(uint8_t tone) { memset(fb_, toByte(tone), W * H / 2); }

void Canvas::pixel(int x, int y, uint8_t tone) { epd_draw_pixel(x, y, toByte(tone), fb_); }

void Canvas::hline(int x, int y, int len, uint8_t tone) {
    if (len < 0) { x += len; len = -len; }
    for (int i = 0; i < len; i++) pixel(x + i, y, tone);
}

void Canvas::vline(int x, int y, int len, uint8_t tone) {
    if (len < 0) { y += len; len = -len; }
    for (int i = 0; i < len; i++) pixel(x, y + i, tone);
}

void Canvas::line(int x0, int y0, int x1, int y1, uint8_t tone, int thickness) {
    if (thickness <= 1) {
        // Bresenham
        int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;
        for (;;) {
            pixel(x0, y0, tone);
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
        return;
    }
    // Thick line: a quad made of two triangles, with round caps.
    float ang = atan2f((float)(y1 - y0), (float)(x1 - x0));
    float ox = -sinf(ang) * thickness / 2.0f, oy = cosf(ang) * thickness / 2.0f;
    int ax = lroundf(x0 + ox), ay = lroundf(y0 + oy);
    int bx = lroundf(x0 - ox), by = lroundf(y0 - oy);
    int cx = lroundf(x1 - ox), cy = lroundf(y1 - oy);
    int dx = lroundf(x1 + ox), dy = lroundf(y1 + oy);
    fillTriangle(ax, ay, bx, by, cx, cy, tone);
    fillTriangle(ax, ay, cx, cy, dx, dy, tone);
    fillCircle(x0, y0, thickness / 2, tone);
    fillCircle(x1, y1, thickness / 2, tone);
}

void Canvas::rect(int x, int y, int w, int h, uint8_t tone) {
    hline(x, y, w, tone);
    hline(x, y + h - 1, w, tone);
    vline(x, y, h, tone);
    vline(x + w - 1, y, h, tone);
}

void Canvas::fillRect(int x, int y, int w, int h, uint8_t tone) {
    for (int j = 0; j < h; j++) hline(x, y + j, w, tone);
}

void Canvas::roundRect(int x, int y, int w, int h, int r, uint8_t tone) {
    hline(x + r, y, w - 2 * r, tone);
    hline(x + r, y + h - 1, w - 2 * r, tone);
    vline(x, y + r, h - 2 * r, tone);
    vline(x + w - 1, y + r, h - 2 * r, tone);
    // corner arcs
    for (int i = 0; i <= r; i++) {
        int j = lroundf(sqrtf((float)(r * r - i * i)));
        int cxL = x + r, cxR = x + w - 1 - r, cyT = y + r, cyB = y + h - 1 - r;
        pixel(cxL - i, cyT - j, tone); pixel(cxL - j, cyT - i, tone);
        pixel(cxR + i, cyT - j, tone); pixel(cxR + j, cyT - i, tone);
        pixel(cxL - i, cyB + j, tone); pixel(cxL - j, cyB + i, tone);
        pixel(cxR + i, cyB + j, tone); pixel(cxR + j, cyB + i, tone);
    }
}

void Canvas::fillRoundRect(int x, int y, int w, int h, int r, uint8_t tone) {
    fillRect(x, y + r, w, h - 2 * r, tone);
    for (int j = 0; j < r; j++) {
        int dy = r - j;
        int inset = r - lroundf(sqrtf((float)(r * r - dy * dy)));
        hline(x + inset, y + j, w - 2 * inset, tone);
        hline(x + inset, y + h - 1 - j, w - 2 * inset, tone);
    }
}

void Canvas::circle(int cx, int cy, int r, uint8_t tone) { ring(cx, cy, r, 1, tone); }

void Canvas::fillCircle(int cx, int cy, int r, uint8_t tone) {
    for (int dy = -r; dy <= r; dy++) {
        int half = (int)floorf(sqrtf((float)(r * r - dy * dy)) + 0.5f);
        hline(cx - half, cy + dy, 2 * half + 1, tone);
    }
}

void Canvas::ring(int cx, int cy, int r, int thickness, uint8_t tone) {
    float outer = r + 0.5f, inner = r + 0.5f - thickness;
    for (int dy = -r - 1; dy <= r + 1; dy++)
        for (int dx = -r - 1; dx <= r + 1; dx++) {
            float d = sqrtf((float)(dx * dx + dy * dy));
            if (d <= outer && d > inner) pixel(cx + dx, cy + dy, tone);
        }
}

void Canvas::fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint8_t tone) {
    // sort by y
    if (y0 > y1) { int t; t = y0; y0 = y1; y1 = t; t = x0; x0 = x1; x1 = t; }
    if (y1 > y2) { int t; t = y1; y1 = y2; y2 = t; t = x1; x1 = x2; x2 = t; }
    if (y0 > y1) { int t; t = y0; y0 = y1; y1 = t; t = x0; x0 = x1; x1 = t; }
    if (y2 == y0) {
        int lo = x0, hi = x0;
        if (x1 < lo) lo = x1; if (x1 > hi) hi = x1;
        if (x2 < lo) lo = x2; if (x2 > hi) hi = x2;
        hline(lo, y0, hi - lo + 1, tone);
        return;
    }
    for (int y = y0; y <= y2; y++) {
        float xa = x0 + (float)(x2 - x0) * (y - y0) / (y2 - y0);
        float xb;
        if (y < y1) xb = (y1 == y0) ? x1 : x0 + (float)(x1 - x0) * (y - y0) / (y1 - y0);
        else        xb = (y2 == y1) ? x1 : x1 + (float)(x2 - x1) * (y - y1) / (y2 - y1);
        if (xa > xb) { float t = xa; xa = xb; xb = t; }
        int a = lroundf(xa), b = lroundf(xb);
        hline(a, y, b - a + 1, tone);
    }
}

void Canvas::dottedHline(int x, int y, int len, int gap, uint8_t tone) {
    for (int i = 0; i < len; i += gap) pixel(x + i, y, tone);
}

int Canvas::textWidth(const GFXfont &font, const char *s) {
    int32_t x = 0, y = 100, x1, y1, w, h;
    get_text_bounds(&font, s, &x, &y, &x1, &y1, &w, &h, nullptr);
    return x;   // advance, not ink box, so alignment is stable
}

int Canvas::text(const GFXfont &font, int x, int baseline, const char *s,
                 Align align, uint8_t ink, uint8_t paper) {
    if (!s || !*s) return x;
    int w = textWidth(font, s);
    if (align == Align::Center) x -= w / 2;
    else if (align == Align::Right) x -= w;
    FontProperties props = {};
    props.fg_color = ink;
    props.bg_color = paper;
    props.fallback_glyph = '?';
    int32_t cx = x, cy = baseline;
    write_mode(&font, s, &cx, &cy, fb_, BLACK_ON_WHITE, &props);
    return cx;
}
