// Renders the dashboard with made-up sample data to a greyscale image, using
// the same screen code as the firmware. See tools/preview/README.md.
#include "screen.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

static Weather sample(time_t now) {
    Weather w = {};
    w.observedAt = now;
    w.temp = 72.4f; w.feelsLike = 70.1f; w.humidity = 34; w.pressure = 1013.2f; w.uvIndex = 6.2f;
    w.windSpeed = 8.3f; w.windGust = 15.1f; w.windFrom = 315;
    w.sky = Sky::FewClouds; w.isNight = false;
    strcpy(w.condition, "scattered clouds");
    w.todayLow = 63; w.todayHigh = 81;
    w.sunrise = now - 4 * 3600 + 9 * 60;
    w.sunset = now + 8 * 3600 - 3 * 60;
    w.moonPhase = 0.14f;   // waxing crescent - the widest phase name
    w.hourCount = Weather::kHours;
    for (int i = 0; i < w.hourCount; i++) {
        w.hours[i].at = now + i * 3600;
        w.hours[i].temp = 72 + 9 * sinf((i - 1) * 3.14159f / 12.0f) - (i > 14 ? (i - 14) * 0.8f : 0);
        w.hours[i].rainChance = (i >= 8 && i <= 13) ? 0.15f + 0.1f * (i - 8) : 0;
    }
    const Sky skies[7] = { Sky::Rain, Sky::Clouds, Sky::Clear, Sky::Storm, Sky::FewClouds, Sky::Fog, Sky::Clear };
    const float hi[7] = { 77, 79, 84, 80, 76, 71, 74 }, lo[7] = { 61, 60, 64, 66, 59, 57, 58 };
    const float rain[7] = { 0.6f, 0.2f, 0, 0.4f, 0.1f, 0, 0 };
    w.dayCount = Weather::kDays;
    for (int i = 0; i < w.dayCount; i++)
        w.days[i] = { now + (i + 1) * 86400, lo[i], hi[i], rain[i], skies[i] };
    return w;
}

int main(int argc, char **argv) {
    const char *out = argc > 1 ? argv[1] : "preview.pgm";
    setenv("TZ", "MST7", 1);
    tzset();
    struct tm tm = {};
    tm.tm_year = 2026 - 1900; tm.tm_mon = 9; tm.tm_mday = 2; tm.tm_hour = 10; tm.tm_min = 15;
    time_t now = mktime(&tm);

    std::vector<uint8_t> fb(EPD_WIDTH * EPD_HEIGHT / 2);
    Canvas c(fb.data());
    Weather w = sample(now);
    DeviceStatus st = { now, true, -58, 3.98f, 68.4f };
    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "--offline")) st.fresh = false;
        if (!strcmp(argv[i], "--no-probe")) st.probeTemp = NAN;
    }
    drawWeatherScreen(c, w, st);
    drawUpdateStamp(c, w, st);

    FILE *f = fopen(out, "wb");
    fprintf(f, "P5\n%d %d\n255\n", EPD_WIDTH, EPD_HEIGHT);
    for (int y = 0; y < EPD_HEIGHT; y++)
        for (int x = 0; x < EPD_WIDTH; x++) {
            uint8_t b = fb[y * EPD_WIDTH / 2 + x / 2];
            uint8_t v = (x & 1) ? (b >> 4) : (b & 0x0F);
            fputc(v * 17, f);
        }
    fclose(f);
    printf("wrote %s\n", out);
    return 0;
}
