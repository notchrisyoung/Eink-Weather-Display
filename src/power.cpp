#include "power.h"
#include "config.h"
#include <Arduino.h>
#include <esp_sleep.h>

namespace power {

float batteryVolts() {
    analogSetPinAttenuation(PIN_BATTERY_ADC, ADC_11db);
    uint32_t mv = 0;
    const int samples = 16;
    for (int i = 0; i < samples; i++) mv += analogReadMilliVolts(PIN_BATTERY_ADC);   // factory-calibrated
    return mv / (float)samples * BATTERY_DIVIDER / 1000.0f;
}

static bool inQuietWindow(int minuteOfDay) {
    if (QUIET_FROM_MIN == QUIET_UNTIL_MIN) return false;
    if (QUIET_FROM_MIN < QUIET_UNTIL_MIN)
        return minuteOfDay >= QUIET_FROM_MIN && minuteOfDay < QUIET_UNTIL_MIN;
    return minuteOfDay >= QUIET_FROM_MIN || minuteOfDay < QUIET_UNTIL_MIN;   // spans midnight
}

uint32_t secondsUntilNextUpdate(time_t now) {
    struct tm tm;
    localtime_r(&now, &tm);
    int secOfDay = tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;

    // Next slot boundary strictly in the future.
    int slot = UPDATE_EVERY_MIN * 60;
    int next = (secOfDay / slot + 1) * slot;

    // Walk forward past any slots that fall in the quiet window.
    for (int guard = 0; guard < 24 * 60 / UPDATE_EVERY_MIN + 1; guard++) {
        if (!inQuietWindow((next / 60) % (24 * 60))) break;
        next += slot;
    }
    // The ESP32's sleep timer tends to run a little fast; aim a few seconds
    // past the boundary so the clock reads :00, not :59.
    return (uint32_t)(next - secOfDay) + 3;
}

void sleepFor(uint32_t seconds) {
    Serial.printf("[power] sleeping %lu s\n", (unsigned long)seconds);
    Serial.flush();
    esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);
    esp_deep_sleep_start();
}

}  // namespace power
