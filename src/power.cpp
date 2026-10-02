#include "power.h"
#include "config.h"
#include <Arduino.h>
#include <esp_sleep.h>
#include <algorithm>

namespace power {

RTC_DATA_ATTR static float smoothedVolts = 0;

static float sampleVolts() {
    analogSetPinAttenuation(PIN_BATTERY_ADC, ADC_11db);
    analogReadMilliVolts(PIN_BATTERY_ADC);   // first conversion after wake is often off; discard it
    const int n = 31;
    uint16_t mv[n];
    for (int i = 0; i < n; i++) {
        mv[i] = analogReadMilliVolts(PIN_BATTERY_ADC);   // uses the chip's factory calibration
        delayMicroseconds(200);
    }
    std::nth_element(mv, mv + n / 2, mv + n);   // median rejects spikes
    return mv[n / 2] * BATTERY_DIVIDER * BATTERY_CALIBRATION / 1000.0f;
}

float batteryVolts() {
    float v = sampleVolts();
    // Smooth across wakes so the percentage doesn't wander. A jump up of more
    // than 0.1 V means it's been charged, so start over from the new reading.
    if (smoothedVolts < 1.0f || v > smoothedVolts + 0.1f || v < 1.0f) smoothedVolts = v;
    else smoothedVolts = smoothedVolts * 0.7f + v * 0.3f;
    Serial.printf("[power] battery %.3f V (smoothed %.3f V)\n", v, smoothedVolts);
    return smoothedVolts;
}

struct Window { int first, last, every; };   // HHMM, HHMM, minutes
static const Window kWindows[] = UPDATE_WINDOWS;

static int toMinutes(int hhmm) { return (hhmm / 100) * 60 + hhmm % 100; }

static bool isUpdateMinute(int m) {
    for (const Window &w : kWindows) {
        int a = toMinutes(w.first), b = toMinutes(w.last);
        if (m >= a && m <= b && (m - a) % w.every == 0) return true;
    }
    return false;
}

bool inUpdateWindow(time_t now) {
    struct tm tm;
    localtime_r(&now, &tm);
    int m = tm.tm_hour * 60 + tm.tm_min;
    for (const Window &w : kWindows)
        if (m >= toMinutes(w.first) && m <= toMinutes(w.last)) return true;
    return false;
}

uint32_t secondsUntilNextUpdate(time_t now) {
    struct tm tm;
    localtime_r(&now, &tm);
    int secOfDay = tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;
    // Look ahead minute by minute, up to two days, for the next slot. The 30 s
    // margin means a wake that came a moment early (5:59:58) counts as the
    // 6:00 update instead of scheduling another wake two seconds later.
    for (int m = (secOfDay + 30) / 60 + 1; m < 2 * 24 * 60; m++) {
        if (isUpdateMinute(m % (24 * 60))) {
            // The ESP32's sleep timer tends to run a little fast; aim a few
            // seconds past the boundary so the clock reads :00, not :59.
            return (uint32_t)(m * 60 - secOfDay) + 3;
        }
    }
    return 3600;   // no windows configured - check back hourly
}

void sleepFor(uint32_t seconds) {
    Serial.printf("[power] sleeping %lu s\n", (unsigned long)seconds);
    Serial.flush();
    esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);
    esp_deep_sleep_start();
}

}  // namespace power
