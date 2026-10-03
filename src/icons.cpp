#include "icons.h"
#include <math.h>
#include "moon_image.h"

static const float kPi = 3.14159265f;

// ---------------------------------------------------------------------------
// Building blocks. Everything is scaled from a unit `s` = size / 100 so the
// same art works for the 150 px "now" icon and the 56 px forecast icons.
// ---------------------------------------------------------------------------

// Smooth union: like fminf, but blends the two shapes over a distance k, which
// rounds the creases between them and keeps the interior free of seams.
static inline float smoothMin(float a, float b, float k) {
    float h = fmaxf(k - fabsf(a - b), 0.0f) / k;
    return fminf(a, b) - h * h * k * 0.25f;
}

// A puffy cloud: three bumps on a flat-bottomed base. Drawn as an ink
// silhouette and then refilled `inset` pixels in, which leaves an outline.
// Signed distance helpers (negative inside), in pixels.
static inline float sdCircle(float x, float y, float cx, float cy, float r) {
    return sqrtf((x - cx) * (x - cx) + (y - cy) * (y - cy)) - r;
}
// Horizontal capsule: segment from ax to bx at height cy, rounded to radius r.
static inline float sdCapsule(float x, float y, float ax, float bx, float cy, float r) {
    float px = x < ax ? ax : (x > bx ? bx : x);
    return sqrtf((x - px) * (x - px) + (y - cy) * (y - cy)) - r;
}

// A puffy cloud: three bumps sitting on a rounded base, so the bottom is a
// smooth flat-ish curve with nothing hanging below it. Drawn as one outlined
// shape (anti-aliased), so overlapping clouds layer cleanly.
static void cloud(Canvas &c, int cx, int cy, float s, uint8_t fill) {
    float edge = s > 0.8f ? 3.5f : 2.0f;
    float X = (float)cx, Y = (float)cy;
    auto sdf = [&](float x, float y) {
        float u = (x - X) / s, v = (y - Y) / s;   // unit space
        const float k = 3;                                     // blend radius
        float d = sdCapsule(u, v, -27, 27, 12, 10);            // base
        d = smoothMin(d, sdCircle(u, v, -17, 4, 13), k);       // left bump
        d = smoothMin(d, sdCircle(u, v, 2, -8, 21), k);        // big middle bump
        d = smoothMin(d, sdCircle(u, v, 21, 3, 14), k);        // right bump
        return d * s;                                          // back to pixels
    };
    c.shape(cx - lroundf(40 * s), cy - lroundf(32 * s), cx + lroundf(40 * s), cy + lroundf(25 * s),
            sdf, fill, Tone::Ink, edge);
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
    float X = (float)cx, Y = (float)cy, R = 15 * s;
    c.shape(cx - r - 2, cy - r - 2, cx + r + 2, cy + r + 2,
            [&](float x, float y) { return sdCircle(x, y, X, Y, R); },
            Tone::Faint, Tone::Ink, s > 0.8f ? 3.5f : 2.0f);
}

// Crescent moon: a disc with an offset disc taken out of it, outlined along
// both curves.
static void crescent(Canvas &c, int cx, int cy, float s) {
    float edge = s > 0.8f ? 3.5f : 2.0f;
    float X = (float)cx, Y = (float)cy;
    auto sdf = [&](float x, float y) {
        float u = (x - X) / s, v = (y - Y) / s;
        float outer = sdCircle(u, v, 0, 0, 20);
        float bite = sdCircle(u, v, 10, -7, 16);
        return fmaxf(outer, -bite) * s;   // inside the disc and outside the bite
    };
    c.shape(cx - lroundf(23 * s), cy - lroundf(23 * s), cx + lroundf(23 * s), cy + lroundf(23 * s),
            sdf, Tone::Faint, Tone::Ink, edge);
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
        if (night) crescent(c, cx, cy, s * 1.4f);
        else sun(c, cx, cy, s * 1.25f);
        break;
    case Sky::FewClouds:
        if (night) crescent(c, cx + lroundf(12 * s), cy - lroundf(22 * s), s * 0.9f);
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
        // one streak, one flake, one streak - evenly spaced under the cloud
        c.line(cx - lroundf(16 * s), cy + lroundf(16 * s), cx - lroundf(22 * s), cy + lroundf(30 * s), Tone::Ink, t);
        flake(c, cx, cy + lroundf(24 * s), s);
        c.line(cx + lroundf(20 * s), cy + lroundf(16 * s), cx + lroundf(14 * s), cy + lroundf(30 * s), Tone::Ink, t);
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
