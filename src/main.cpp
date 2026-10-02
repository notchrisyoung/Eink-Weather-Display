// E-ink weather dashboard for the LilyGo T5 4.7" (ESP32 + 960x540 e-paper).
//
// Each wake: join Wi-Fi, sync the clock, download the forecast, read the
// local probe, draw the dashboard, then deep-sleep until the next slot.
// The last good forecast is kept in RTC memory so a failed update can still
// redraw the screen with an "offline" note instead of going blank.
#include <Arduino.h>
#include "epd_driver.h"
#include "config.h"
#include "credentials.h"
#include "canvas.h"
#include "net.h"
#include "owm.h"
#include "power.h"
#include "probe.h"
#include "screen.h"

RTC_DATA_ATTR static Weather lastForecast;
RTC_DATA_ATTR static bool haveForecast = false;
RTC_DATA_ATTR static uint8_t failures = 0;

static void showOnPanel(void (*draw)(Canvas &, void *), void *ctx) {
    uint8_t *fb = (uint8_t *)ps_calloc(1, EPD_WIDTH * EPD_HEIGHT / 2);
    if (!fb) {
        Serial.println("[main] framebuffer allocation failed - is PSRAM enabled?");
        return;
    }
    epd_init();
    epd_poweron();
    delay(10);   // let the panel supply (and battery sense divider) settle
    Canvas canvas(fb);
    draw(canvas, ctx);
    epd_clear();
    epd_draw_grayscale_image(epd_full_screen(), fb);
    epd_poweroff_all();
    free(fb);
}

struct Frame {
    const Weather *weather;
    DeviceStatus status;
};

void setup() {
    Serial.begin(115200);
    delay(50);
    Serial.println("\n[main] wake");

    bool online = net::connect(WIFI_SSID, WIFI_PASSWORD, 20000);
    int rssi = net::rssi();
    bool clockOk = online && net::syncClock(WX_TIMEZONE, 10000);
    if (!clockOk) net::useTimeZone(WX_TIMEZONE);   // fall back to the RTC

    Weather fresh;
    bool gotForecast = online && owm::fetch(fresh);
    net::shutdown();

    if (gotForecast) {
        lastForecast = fresh;
        haveForecast = true;
        failures = 0;
    } else if (failures < 255) {
        failures++;
    }

    time_t now = time(nullptr);
    bool clockValid = now > 1700000000;   // anything before Nov 2023 means "never set"

    // Redraw on success, and on the first failure (to show the offline note).
    // Further failures leave the panel alone - e-paper keeps its image for free.
    if (gotForecast || failures == 1 || !haveForecast) {
        Frame frame;
        frame.weather = &lastForecast;
        frame.status.now = clockValid ? now : lastForecast.observedAt;
        frame.status.fresh = gotForecast;
        frame.status.wifiRssi = rssi;
        frame.status.batteryVolts = 0;   // read while the panel is powered
        frame.status.probeTemp = probe::readTemperature();

        showOnPanel([](Canvas &c, void *p) {
            Frame *f = (Frame *)p;
            f->status.batteryVolts = power::batteryVolts();
            if (haveForecast) drawWeatherScreen(c, *f->weather, f->status);
            else drawMessageScreen(c, "Waiting for weather", "Check Wi-Fi and the API key, retrying in a few minutes");
        }, &frame);
    }

    uint32_t sleepSec;
    if (!gotForecast) {
        // Back off: 5, 10, 20 ... minutes, but never longer than a normal update.
        uint32_t backoff = (uint32_t)RETRY_AFTER_FAIL_MIN * 60u << (failures > 4 ? 4 : failures - 1);
        uint32_t normal = clockValid ? power::secondsUntilNextUpdate(now) : backoff;
        sleepSec = backoff < normal ? backoff : normal;
    } else {
        sleepSec = power::secondsUntilNextUpdate(now);
    }
    Serial.printf("[main] awake %lu ms\n", (unsigned long)millis());
    power::sleepFor(sleepSec);
}

void loop() {
    // Not reached: setup() always ends in deep sleep.
}
