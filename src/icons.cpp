#include "icons.h"
#include <math.h>
#include "moon_image.h"

static const float kPi = 3.14159265f;

// ---------------------------------------------------------------------------
// Building blocks. Everything is scaled from a unit `s` = size / 100 so the
// same art works for the 150 px "now" icon and the 56 px forecast icons.
// ---------------------------------------------------------------------------

// A puffy cloud: three bumps on a flat-bottomed base. Drawn as an ink
// silhouette and then refilled `inset` pixels in, which leaves an outline.
static void cloudShape(Canvas &c, int cx, int cy, float s, int inset, uint8_t tone) {
    struct Bump { float x, y, r; };
    const Bump bumps[] = { {-24, 6, 17}, {-3, -9, 24}, {22, 3, 18} };
    for (const Bump &b : bumps)
        c.fillCircle(cx + lroundf(b.x * s), cy + lroundf(b.y * s), lroundf(b.r * s) - inset, tone);
    int left = cx + lroundf(-24 * s), right = cx + lroundf(22 * s);
    int top = cy + lroundf(4 * s), bottom = cy + lroundf(21 * s);
    c.fillRect(left, top, right - left, bottom - top - inset, tone);
}

static void cloud(Canvas &c, int cx, int cy, float s, uint8_t fill) {
    int edge = s > 0.8f ? 4 : 2;
    cloudShape(c, cx, cy, s, 0, Tone::Ink);
    cloudShape(c, cx, cy, s, edge, fill);
}

static void sun(Canvas &c, int cx, int cy, float s) {
    int r = lroundf(15 * s);
    int thick = s > 0.8f ? 5 : 2;
    for (int i = 0; i < 8; i++) {
        float a = i * kPi / 4;
        int x0 = cx + lroundf(cosf(a) * 23 * s), y0 = cy + lroundf(sinf(a) * 23 * s);
        int x1 = cx + lroundf(cosf(a) * 33 * s), y1 = cy + lroundf(sinf(a) * 33 * s);
        c.line(x0, y0, x1, y1, Tone::Ink, thick);
    }
    c.fillCircle(cx, cy, r, Tone::Ink);
    c.fillCircle(cx, cy, r - thick, Tone::Faint);
}

static void crescent(Canvas &c, int cx, int cy, float s, uint8_t behind) {
    int r = lroundf(20 * s);
    c.fillCircle(cx, cy, r, Tone::Ink);
    c.fillCircle(cx, cy, r - (s > 0.8f ? 4 : 2), Tone::Faint);
    c.fillCircle(cx + lroundf(11 * s), cy - lroundf(8 * s), lroundf(17 * s), behind);
}

static void rainStreaks(Canvas &c, int cx, int top, float s, int count, int thick) {
    for (int i = 0; i < count; i++) {
        int x = cx + lroundf((-18 + i * 36.0f / (count - 1 > 0 ? count - 1 : 1)) * s);
        c.line(x, top, x - lroundf(6 * s), top + lroundf(14 * s), Tone::Ink, thick);
    }
}

static void drizzleDots(Canvas &c, int cx, int top, float s) {
    for (int i = 0; i < 3; i++) {
        int x = cx + lroundf((-16 + i * 16) * s);
        c.fillCircle(x, top + lroundf((i % 2 ? 10 : 4) * s), lroundf(3 * s) + 1, Tone::Ink);
    }
}

static void flake(Canvas &c, int cx, int cy, float s) {
    int r = lroundf(7 * s) + 1;
    int t = s > 0.8f ? 2 : 1;
    for (int i = 0; i < 3; i++) {
        float a = i * kPi / 3;
        c.line(cx - lroundf(cosf(a) * r), cy - lroundf(sinf(a) * r),
               cx + lroundf(cosf(a) * r), cy + lroundf(sinf(a) * r), Tone::Ink, t);
    }
}

static void bolt(Canvas &c, int cx, int top, float s) {
    auto P = [&](float x, float y, int &ox, int &oy) { ox = cx + lroundf(x * s); oy = top + lroundf(y * s); };
    int ax, ay, bx, by, cx2, cy2, dx, dy, ex, ey, fx, fy;
    P(2, -4, ax, ay); P(-10, 14, bx, by); P(0, 14, cx2, cy2);
    P(-6, 30, dx, dy); P(12, 8, ex, ey); P(2, 8, fx, fy);
    c.fillTriangle(ax, ay, bx, by, cx2, cy2, Tone::Ink);
    c.fillTriangle(ax, ay, cx2, cy2, fx, fy, Tone::Ink);
    c.fillTriangle(fx, fy, ex, ey, dx, dy, Tone::Ink);
    c.fillTriangle(cx2, cy2, fx, fy, dx, dy, Tone::Ink);
}

static void fogBands(Canvas &c, int cx, int cy, float s) {
    int t = s > 0.8f ? 5 : 2;
    const float rows[][2] = { {-30, 22}, {-22, 30}, {-34, 18}, {-18, 32} };
    for (int i = 0; i < 4; i++) {
        int y = cy + lroundf((-15 + i * 10) * s);
        c.line(cx + lroundf(rows[i][0] * s), y, cx + lroundf(rows[i][1] * s), y,
               i % 2 ? Tone::Mid : Tone::Ink, t);
    }
}

// ---------------------------------------------------------------------------

void drawSkyIcon(Canvas &c, int cx, int cy, int size, Sky sky, bool night) {
    float s = size / 100.0f;
    int t = s > 0.8f ? 4 : 2;
    switch (sky) {
    case Sky::Clear:
        if (night) crescent(c, cx, cy, s * 1.4f, Tone::Paper);
        else sun(c, cx, cy, s * 1.25f);
        break;
    case Sky::FewClouds:
        if (night) crescent(c, cx + lroundf(14 * s), cy - lroundf(14 * s), s, Tone::Paper);
        else sun(c, cx + lroundf(14 * s), cy - lroundf(14 * s), s);
        cloud(c, cx - lroundf(4 * s), cy + lroundf(10 * s), s * 0.95f, Tone::Paper);
        break;
    case Sky::Clouds:
        cloud(c, cx + lroundf(12 * s), cy - lroundf(10 * s), s * 0.75f, Tone::Faint);
        cloud(c, cx - lroundf(6 * s), cy + lroundf(8 * s), s, Tone::Paper);
        break;
    case Sky::Overcast:
        cloud(c, cx + lroundf(12 * s), cy - lroundf(10 * s), s * 0.75f, Tone::Soft);
        cloud(c, cx - lroundf(6 * s), cy + lroundf(8 * s), s, Tone::Faint);
        break;
    case Sky::Drizzle:
        cloud(c, cx, cy - lroundf(12 * s), s, Tone::Whisper);
        drizzleDots(c, cx, cy + lroundf(16 * s), s);
        break;
    case Sky::Rain:
        cloud(c, cx, cy - lroundf(12 * s), s, Tone::Faint);
        rainStreaks(c, cx, cy + lroundf(16 * s), s, 4, t);
        break;
    case Sky::Storm:
        cloud(c, cx, cy - lroundf(14 * s), s, Tone::Soft);
        bolt(c, cx, cy + lroundf(10 * s), s);
        break;
    case Sky::Snow:
        cloud(c, cx, cy - lroundf(12 * s), s, Tone::Whisper);
        flake(c, cx - lroundf(16 * s), cy + lroundf(22 * s), s);
        flake(c, cx + lroundf(2 * s), cy + lroundf(32 * s), s);
        flake(c, cx + lroundf(20 * s), cy + lroundf(20 * s), s);
        break;
    case Sky::Sleet:
        cloud(c, cx, cy - lroundf(12 * s), s, Tone::Faint);
        rainStreaks(c, cx - lroundf(8 * s), cy + lroundf(16 * s), s, 2, t);
        flake(c, cx + lroundf(18 * s), cy + lroundf(26 * s), s);
        break;
    case Sky::Fog:
        fogBands(c, cx, cy, s);
        break;
    }
}

// E-paper renders mid-greys darker than they look on a monitor, so the photo's
// lit side is brightened (roughly gamma 0.55) and the shadow side is lifted a
// little so craters still show in it, while staying clearly darker.
static const uint8_t kLitCurve[16]    = { 0, 3, 5, 6, 7, 8, 9, 10, 11, 11, 12, 13, 13, 14, 14, 15 };
static const uint8_t kShadowCurve[16] = { 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6 };

void drawMoon(Canvas &c, int x0, int y0, float phase, bool southern) {
    // Photo of the full moon, with the unlit part darkened to match the phase.
    const float r = kMoonSize / 2.0f, centre = (kMoonSize - 1) / 2.0f;
    const float cosT = cosf(2 * kPi * phase);
    const bool waxing = phase < 0.5f;
    for (int y = 0; y < kMoonSize; y++) {
        float dy = y - centre;
        float half = sqrtf(fmaxf(r * r - dy * dy, 0));
        float edge = half * cosT;           // terminator position on this row
        for (int x = 0; x < kMoonSize; x++) {
            float dx = x - centre;
            if (dx * dx + dy * dy > r * r) continue;   // outside the disc: leave the background
            uint8_t v = kMoonPixels[y * kMoonRowBytes + x / 2];
            v = (x & 1) ? (v >> 4) : (v & 0x0F);
            float sx = southern ? -dx : dx;
            bool lit = waxing ? (sx >= edge) : (sx <= -edge);
            c.pixel(x0 + x, y0 + y, lit ? kLitCurve[v] : kShadowCurve[v]);
        }
    }
    // Faint outline so a thin crescent's lit edge doesn't melt into the tile.
    c.ring(x0 + kMoonSize / 2, y0 + kMoonSize / 2, kMoonSize / 2, 1, Tone::Soft);
}

void drawWifiBars(Canvas &c, int x, int baseline, int rssi, uint8_t tone, uint8_t off) {
    int bars = rssi == 0 ? 0 : rssi > -55 ? 4 : rssi > -65 ? 3 : rssi > -75 ? 2 : 1;
    for (int i = 0; i < 4; i++) {
        int h = 5 + i * 4;
        c.fillRect(x + i * 6, baseline - h, 4, h, i < bars ? tone : off);
    }
}

void drawBattery(Canvas &c, int x, int y, int percent, uint8_t tone) {
    const int w = 34, h = 16;
    c.rect(x, y, w, h, tone);
    c.rect(x + 1, y + 1, w - 2, h - 2, tone);
    c.fillRect(x + w, y + 5, 3, h - 10, tone);
    if (percent < 0) return;
    int fill = (w - 6) * (percent > 100 ? 100 : percent) / 100;
    c.fillRect(x + 3, y + 3, fill, h - 6, tone);
}

void drawDrop(Canvas &c, int cx, int cy, int h, uint8_t tone) {
    int r = h / 3;
    c.fillCircle(cx, cy + h / 2 - r, r, tone);
    c.fillTriangle(cx, cy - h / 2, cx - r, cy + h / 2 - r, cx + r, cy + h / 2 - r, tone);
}
