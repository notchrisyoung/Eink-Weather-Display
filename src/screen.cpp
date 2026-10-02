#include "screen.h"
#include "icons.h"
#include "config.h"
#include "fonts/Sans9.h"
#include "fonts/Sans11.h"
#include "fonts/SansBold11.h"
#include "fonts/SansBold14.h"
#include "fonts/SansBold20.h"
#include "fonts/SansBold48.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define DEG "\xC2\xB0"   // UTF-8 degree sign

// ---- Layout grid (960 x 540) ----------------------------------------------
namespace L {
    const int margin = 24;
    const int headerH = 56;
    const int tilesX = 440, tilesY = 70, tileW = 160, tileH = 122, tileGap = 12;
    const int splitY = 336;                 // top half / bottom half
    const int chartX = 24, chartRight = 556;
    const int daysX = 584, daysRight = 936;
}

// ---- Small formatting helpers ----------------------------------------------

static void fmtTemp(char *out, size_t n, float t) {
    if (isnan(t)) { snprintf(out, n, "--"); return; }
    snprintf(out, n, "%d" DEG, (int)lroundf(t));
}

static void fmtClock(char *out, size_t n, time_t t) {
    struct tm tm;
    localtime_r(&t, &tm);
#if CLOCK_24H
    snprintf(out, n, "%02d:%02d", tm.tm_hour, tm.tm_min);
#else
    int h = tm.tm_hour % 12;
    snprintf(out, n, "%d:%02d %s", h ? h : 12, tm.tm_min, tm.tm_hour < 12 ? "AM" : "PM");
#endif
}

static void fmtHourTick(char *out, size_t n, time_t t) {
    struct tm tm;
    localtime_r(&t, &tm);
#if CLOCK_24H
    snprintf(out, n, "%02d", tm.tm_hour);
#else
    int h = tm.tm_hour % 12;
    snprintf(out, n, "%d%s", h ? h : 12, tm.tm_hour < 12 ? "a" : "p");
#endif
}

static const char *compassPoint(float deg) {
    static const char *names[16] = { "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE",
                                     "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW" };
    int i = (int)floorf(fmodf(deg + 11.25f + 360.0f, 360.0f) / 22.5f);
    return names[i & 15];
}

// Two-line moon phase name, e.g. {"Waxing", "gibbous"}.
static void moonPhaseName(float p, const char **a, const char **b) {
    const char *first[8]  = { "New", "Waxing", "First", "Waxing", "Full", "Waning", "Last", "Waning" };
    const char *second[8] = { "moon", "crescent", "quarter", "gibbous", "moon", "gibbous", "quarter", "crescent" };
    int i = (int)floorf(fmodf(p, 1.0f) * 8.0f + 0.5f) & 7;
    *a = first[i];
    *b = second[i];
}

static const char *uvWord(float uv) {
    if (uv < 3) return "Low";
    if (uv < 6) return "Moderate";
    if (uv < 8) return "High";
    if (uv < 11) return "Very high";
    return "Extreme";
}

int batteryPercent(float v) {
    if (v < 1.0f) return -1;
    // Typical single-cell LiPo resting voltage curve.
    static const float volts[] = { 3.30f, 3.45f, 3.60f, 3.68f, 3.74f, 3.79f, 3.85f, 3.93f, 4.02f, 4.10f, 4.18f };
    if (v <= volts[0]) return 0;
    if (v >= volts[10]) return 100;
    int i = 1;
    while (v >= volts[i]) i++;
    float pct = (i - 1 + (v - volts[i - 1]) / (volts[i] - volts[i - 1])) * 10.0f;
    // A voltage reading can't honestly resolve better than ~5%, so don't pretend to.
    return (int)lroundf(pct / 5.0f) * 5;
}

// ---- Sections ---------------------------------------------------------------

static void header(Canvas &c, const Weather &w, const DeviceStatus &st) {
    c.fillRect(0, 0, Canvas::W, L::headerH, Tone::Ink);
    c.text(SansBold20, L::margin, 39, WX_PLACE_NAME, Align::Left, Tone::Paper, Tone::Ink);

    char buf[64];
    struct tm tm;
    time_t shown = st.fresh ? st.now : w.observedAt;
    localtime_r(&shown, &tm);
    static const char *days[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
    static const char *months[12] = { "January", "February", "March", "April", "May", "June", "July",
                                      "August", "September", "October", "November", "December" };
    snprintf(buf, sizeof buf, "%s, %s %d", days[tm.tm_wday], months[tm.tm_mon], tm.tm_mday);
    c.text(SansBold14, Canvas::W / 2, 37, buf, Align::Center, Tone::Paper, Tone::Ink);

    // Battery sits at the far right; the update stamp (drawn separately) goes left of it.
    int pct = batteryPercent(st.batteryVolts);
    drawBattery(c, Canvas::W - L::margin - 37, 20, pct, Tone::Paper);
    if (pct >= 0) {
        snprintf(buf, sizeof buf, "%d%%", pct);
        c.text(Sans9, Canvas::W - L::margin - 45, 34, buf, Align::Right, Tone::Paper, Tone::Ink);
    }
}

// x where the battery block starts, so the stamp can sit just left of it.
static int batteryBlockLeft(Canvas &c, const DeviceStatus &st) {
    int x = Canvas::W - L::margin - 45;
    int pct = batteryPercent(st.batteryVolts);
    if (pct >= 0) {
        char buf[16];
        snprintf(buf, sizeof buf, "%d%%", pct);
        x -= c.textWidth(Sans9, buf);
    }
    return x;
}

static void nowPanel(Canvas &c, const Weather &w) {
    drawSkyIcon(c, 112, 180, 150, w.sky, w.isNight);

    char buf[48];
    fmtTemp(buf, sizeof buf, w.temp);
    c.text(SansBold48, 210, 206, buf);

    char t[16];
    fmtTemp(t, sizeof t, w.feelsLike);
    snprintf(buf, sizeof buf, "Feels like %s", t);
    c.text(Sans11, 214, 242, buf, Align::Left, Tone::Dark);

    // Condition, capitalised: "scattered clouds" -> "Scattered clouds"
    strncpy(buf, w.condition, sizeof buf - 1);
    buf[sizeof buf - 1] = 0;
    buf[0] = (char)toupper((unsigned char)buf[0]);
    c.text(SansBold14, L::margin + 6, 292, buf);

    char lo[16], hi[16];
    fmtTemp(hi, sizeof hi, w.todayHigh);
    fmtTemp(lo, sizeof lo, w.todayLow);
    snprintf(buf, sizeof buf, "High %s   Low %s", hi, lo);
    c.text(Sans11, L::margin + 6, 322, buf, Align::Left, Tone::Dark);
}

// A grey card with a small caption; returns its top-left through x/y.
static void tile(Canvas &c, int col, int row, const char *caption, int *x, int *y) {
    *x = L::tilesX + col * (L::tileW + L::tileGap);
    *y = L::tilesY + row * (L::tileH + L::tileGap);
    c.fillRoundRect(*x, *y, L::tileW, L::tileH, 10, Tone::Faint);
    c.text(Sans9, *x + 14, *y + 26, caption, Align::Left, Tone::Dark, Tone::Faint);
}

static void tiles(Canvas &c, const Weather &w, const DeviceStatus &st) {
    int x, y;
    char buf[40], small[24];

    // Wind
    tile(c, 0, 0, "Wind", &x, &y);
    float speed = WX_IMPERIAL ? w.windSpeed : w.windSpeed * 3.6f;
    float gust = WX_IMPERIAL ? w.windGust : w.windGust * 3.6f;
    snprintf(buf, sizeof buf, "%d", (int)lroundf(speed));
    int end = c.text(SansBold20, x + 14, y + 72, buf, Align::Left, Tone::Ink, Tone::Faint);
    c.text(Sans9, end + 4, y + 72, WX_IMPERIAL ? "mph" : "km/h", Align::Left, Tone::Dark, Tone::Faint);
    if (gust > speed + 1) snprintf(buf, sizeof buf, "%s, gust %d", compassPoint(w.windFrom), (int)lroundf(gust));
    else snprintf(buf, sizeof buf, "from %s", compassPoint(w.windFrom));
    c.text(Sans9, x + 14, y + 106, buf, Align::Left, Tone::Dark, Tone::Faint);
    drawWindDial(c, x + 118, y + 52, 28, w.windFrom);

    // Humidity, with a fill bar
    tile(c, 1, 0, "Humidity", &x, &y);
    snprintf(buf, sizeof buf, "%d%%", (int)lroundf(w.humidity));
    c.text(SansBold20, x + 14, y + 72, buf, Align::Left, Tone::Ink, Tone::Faint);
    c.fillRoundRect(x + 14, y + 92, L::tileW - 28, 12, 6, Tone::Paper);
    int fillW = (int)((L::tileW - 28) * fminf(fmaxf(w.humidity, 0), 100) / 100.0f);
    if (fillW > 12) c.fillRoundRect(x + 14, y + 92, fillW, 12, 6, Tone::Mid);

    // Pressure
    tile(c, 2, 0, "Pressure", &x, &y);
    if (WX_IMPERIAL) snprintf(buf, sizeof buf, "%.2f", w.pressure * 0.02953f);
    else snprintf(buf, sizeof buf, "%d", (int)lroundf(w.pressure));
    c.text(SansBold20, x + 14, y + 72, buf, Align::Left, Tone::Ink, Tone::Faint);
    c.text(Sans9, x + 14, y + 106, WX_IMPERIAL ? "inHg" : "hPa", Align::Left, Tone::Dark, Tone::Faint);

    // Sunrise / sunset, with little up/down markers
    tile(c, 0, 1, "Sun", &x, &y);
    fmtClock(small, sizeof small, w.sunrise);
    c.fillTriangle(x + 16, y + 62, x + 30, y + 62, x + 23, y + 52, Tone::Ink);
    c.text(SansBold14, x + 40, y + 66, small, Align::Left, Tone::Ink, Tone::Faint);
    fmtClock(small, sizeof small, w.sunset);
    c.fillTriangle(x + 16, y + 90, x + 30, y + 90, x + 23, y + 100, Tone::Mid);
    c.text(SansBold14, x + 40, y + 102, small, Align::Left, Tone::Ink, Tone::Faint);

    // Moon
    tile(c, 1, 1, "Moon", &x, &y);
    const char *p1, *p2;
    moonPhaseName(w.moonPhase, &p1, &p2);
    c.text(SansBold11, x + 14, y + 70, p1, Align::Left, Tone::Ink, Tone::Faint);
    c.text(SansBold11, x + 14, y + 96, p2, Align::Left, Tone::Ink, Tone::Faint);
    drawMoon(c, x + 128, y + 62, 23, w.moonPhase, SOUTHERN_HEMISPHERE);

    // Local probe if one is fitted, otherwise the UV index
    if (!isnan(st.probeTemp)) {
        tile(c, 2, 1, TEMP_PROBE_LABEL, &x, &y);
        snprintf(buf, sizeof buf, "%.1f" DEG, st.probeTemp);
        c.text(SansBold20, x + 14, y + 72, buf, Align::Left, Tone::Ink, Tone::Faint);
        c.text(Sans9, x + 14, y + 106, "local sensor", Align::Left, Tone::Dark, Tone::Faint);
    } else {
        tile(c, 2, 1, "UV index", &x, &y);
        snprintf(buf, sizeof buf, "%d", (int)lroundf(w.uvIndex));
        c.text(SansBold20, x + 14, y + 72, buf, Align::Left, Tone::Ink, Tone::Faint);
        c.text(Sans9, x + 14, y + 106, uvWord(w.uvIndex), Align::Left, Tone::Dark, Tone::Faint);
    }
}

static void hourlyChart(Canvas &c, const Weather &w) {
    const int top = L::splitY + 58, bottom = 494;
    const int left = L::chartX + 44, right = L::chartRight;
    c.text(SansBold11, L::chartX, L::splitY + 28, "Next 24 hours");
    // Legend swatch matches the rain columns
    const char *legend = "chance of rain";
    c.text(Sans9, right, L::splitY + 27, legend, Align::Right, Tone::Dark);
    c.fillRect(right - c.textWidth(Sans9, legend) - 22, L::splitY + 14, 14, 14, Tone::Faint);

    int n = w.hourCount;
    if (n < 2) return;

    float lo = 1e9f, hi = -1e9f;
    for (int i = 0; i < n; i++) { lo = fminf(lo, w.hours[i].temp); hi = fmaxf(hi, w.hours[i].temp); }
    // Pad the range so a flat day doesn't turn into a wild zig-zag.
    float span = fmaxf(hi - lo, 8.0f);
    float mid = (hi + lo) / 2;
    lo = mid - span / 2 - 1;
    hi = mid + span / 2 + 1;

    float step = (float)(right - left) / n;
    // Rain-chance columns behind everything
    for (int i = 0; i < n; i++) {
        int h = (int)((bottom - top) * w.hours[i].rainChance);
        if (h > 0) c.fillRect(left + (int)(i * step) + 1, bottom - h, (int)step - 2, h, Tone::Faint);
    }
    // Frame and labels
    c.dottedHline(left, top, right - left, 4, Tone::Mid);
    c.hline(left, bottom, right - left, Tone::Mid);
    char buf[16];
    fmtTemp(buf, sizeof buf, hi);
    c.text(Sans9, left - 8, top + 6, buf, Align::Right, Tone::Dark);
    fmtTemp(buf, sizeof buf, lo);
    c.text(Sans9, left - 8, bottom, buf, Align::Right, Tone::Dark);

    // Temperature line
    int px = 0, py = 0;
    for (int i = 0; i < n; i++) {
        int x = left + (int)(i * step + step / 2);
        int y = bottom - (int)((w.hours[i].temp - lo) / (hi - lo) * (bottom - top));
        if (i) c.line(px, py, x, y, Tone::Ink, 3);
        px = x; py = y;
    }
    // Highlight the warmest hour
    int warmest = 0;
    for (int i = 1; i < n; i++) if (w.hours[i].temp > w.hours[warmest].temp) warmest = i;
    {
        int x = left + (int)(warmest * step + step / 2);
        int y = bottom - (int)((w.hours[warmest].temp - lo) / (hi - lo) * (bottom - top));
        c.fillCircle(x, y, 6, Tone::Ink);
        c.fillCircle(x, y, 3, Tone::Paper);
        fmtTemp(buf, sizeof buf, w.hours[warmest].temp);
        c.text(SansBold11, x, y - 12, buf, Align::Center);
    }
    // Hour ticks every 3 hours
    for (int i = 0; i < n; i += 3) {
        int x = left + (int)(i * step + step / 2);
        c.vline(x, bottom, 5, Tone::Mid);
        if (i == 0) snprintf(buf, sizeof buf, "Now");
        else fmtHourTick(buf, sizeof buf, w.hours[i].at);
        c.text(Sans9, x, bottom + 24, buf, Align::Center, Tone::Dark);
    }
}

static void outlook(Canvas &c, const Weather &w) {
    int n = w.dayCount;
    if (n == 0) return;
    int colW = (L::daysRight - L::daysX) / n;
    for (int i = 0; i < n; i++) {
        const DayOutlook &d = w.days[i];
        int cx = L::daysX + i * colW + colW / 2;
        if (i) c.vline(L::daysX + i * colW, L::splitY + 18, 176, Tone::Faint);

        struct tm tm;
        localtime_r(&d.at, &tm);
        static const char *wd[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
        c.text(SansBold11, cx, L::splitY + 34, wd[tm.tm_wday], Align::Center);
        drawSkyIcon(c, cx, L::splitY + 84, 58, d.sky, false);

        char buf[16];
        fmtTemp(buf, sizeof buf, d.high);
        c.text(SansBold14, cx, L::splitY + 146, buf, Align::Center);
        fmtTemp(buf, sizeof buf, d.low);
        c.text(Sans11, cx, L::splitY + 172, buf, Align::Center, Tone::Mid);
        if (d.rainChance >= 0.1f) {
            snprintf(buf, sizeof buf, "%d%%", (int)lroundf(d.rainChance * 100));
            int tw = c.textWidth(Sans9, buf);
            drawDrop(c, cx - tw / 2 - 6, L::splitY + 192, 12, Tone::Mid);
            c.text(Sans9, cx + 6, L::splitY + 198, buf, Align::Center, Tone::Dark);
        }
    }
}

// ---- Public -------------------------------------------------------------------

void drawWeatherScreen(Canvas &c, const Weather &w, const DeviceStatus &st) {
    c.fill(Tone::Paper);
    header(c, w, st);
    nowPanel(c, w);
    tiles(c, w, st);
    c.hline(L::margin, L::splitY, Canvas::W - 2 * L::margin, Tone::Soft);
    c.vline(L::daysX - 14, L::splitY + 18, 176, Tone::Soft);
    hourlyChart(c, w);
    outlook(c, w);
}

void drawUpdateStamp(Canvas &c, const Weather &w, const DeviceStatus &st) {
    int x = batteryBlockLeft(c, st) - 14 - 22;   // four Wi-Fi bars, 22 px wide
    drawWifiBars(c, x, 36, st.wifiRssi, Tone::Paper, Tone::Mid);
    char when[16], buf[48];
    fmtClock(when, sizeof when, w.observedAt);
    if (st.fresh) snprintf(buf, sizeof buf, "Updated %s", when);
    else          snprintf(buf, sizeof buf, "Offline - data from %s", when);
    c.text(Sans9, x - 12, 34, buf, Align::Right, Tone::Paper, Tone::Ink);
}

void drawMessageScreen(Canvas &c, const char *title, const char *detail) {
    c.fill(Tone::Paper);
    c.fillRect(0, 0, Canvas::W, L::headerH, Tone::Ink);
    c.text(SansBold20, L::margin, 39, WX_PLACE_NAME, Align::Left, Tone::Paper, Tone::Ink);
    drawSkyIcon(c, Canvas::W / 2, 200, 140, Sky::Overcast, false);
    c.text(SansBold20, Canvas::W / 2, 330, title, Align::Center);
    c.text(Sans11, Canvas::W / 2, 372, detail, Align::Center, Tone::Dark);
}
