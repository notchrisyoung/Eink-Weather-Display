// E-ink weather dashboard for the LilyGo T5 4.7" (ESP32 + 960x540 e-paper).
//
// Each wake: read the battery, join Wi-Fi, download the forecast (which also
// sets the clock), read the local probe, draw the dashboard in memory, and
// refresh the panel only if something besides the update time changed.
// Then deep-sleep until the next slot in UPDATE_WINDOWS.
//
// The last good forecast is kept in RTC memory so a failed update can still
// redraw the screen with an "offline" note instead of going blank.
#include <Arduino.h>
#include <rom/crc.h>
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
RTC_DATA_ATTR static uint32_t shownChecksum = 0;   // what's on the panel now (minus the stamp)

static const size_t kFrameBytes = EPD_WIDTH * EPD_HEIGHT / 2;

static bool clockIsSet(time_t t) { return t > 1700000000; }   // before Nov 2023 = never set

void setup() {
    Serial.begin(115200);
    Serial.println("\n[main] wake");
    net::useTimeZone(WX_TIMEZONE);

    // Battery first, while nothing else is drawing current.
    epd_init();
    epd_poweron();
    delay(10);   // panel supply also feeds the battery sense divider
    float battery = power::batteryVolts();
    epd_poweroff();

    bool online = net::connect(WIFI_SSID, WIFI_PASSWORD, 20000);
    int rssi = net::rssi();

    Weather fresh;
    time_t serverTime = 0;
    bool gotForecast = online && owm::fetch(fresh, &serverTime);
    if (clockIsSet(serverTime)) net::setClock(serverTime);
    else if (online && !clockIsSet(time(nullptr))) net::syncClock(10000);   // first boot fallback
    net::shutdown();

    if (gotForecast) {
        lastForecast = fresh;
        haveForecast = true;
        failures = 0;
    } else if (failures < 255) {
        failures++;
    }

    time_t now = time(nullptr);
    bool clockValid = clockIsSet(now);

    // Draw on success, and on the first failure (to show the offline note).
    // Further failures leave the panel alone - e-paper keeps its image for free.
    if (gotForecast || failures == 1 || !haveForecast) {
        DeviceStatus st;
        st.now = clockValid ? now : lastForecast.observedAt;
        st.fresh = gotForecast;
        st.wifiRssi = rssi;
        st.batteryVolts = battery;
        st.probeTemp = probe::readTemperature();

        uint8_t *fb = (uint8_t *)ps_calloc(1, kFrameBytes);
        if (!fb) {
            Serial.println("[main] framebuffer allocation failed - is PSRAM enabled?");
        } else {
            Canvas canvas(fb);
            if (haveForecast) drawWeatherScreen(canvas, lastForecast, st);
            else drawMessageScreen(canvas, "Waiting for weather", "Check Wi-Fi and the API key, retrying in a few minutes");

            // Compare against what's already showing, ignoring the update stamp.
            uint32_t sum = crc32_le(0, fb, kFrameBytes);
            bool changed = sum != shownChecksum || !gotForecast;
            if (changed) {
                if (haveForecast) drawUpdateStamp(canvas, lastForecast, st);
                epd_poweron();
                epd_clear();
                epd_draw_grayscale_image(epd_full_screen(), fb);
                epd_poweroff_all();
                // After an offline frame, force the next good update to redraw.
                shownChecksum = gotForecast ? sum : 0;
                Serial.println("[main] panel refreshed");
            } else {
                Serial.println("[main] nothing changed, panel left as is");
            }
            free(fb);
        }
    }

    uint32_t sleepSec;
    if (!gotForecast) {
        // Back off: 5, 10, 20 ... minutes, but never later than the next normal
        // slot, and no retries overnight - just wait for the morning.
        uint32_t backoff = (uint32_t)RETRY_AFTER_FAIL_MIN * 60u << (failures > 4 ? 4 : failures - 1);
        if (!clockValid) sleepSec = backoff;
        else {
            uint32_t normal = power::secondsUntilNextUpdate(now);
            sleepSec = (power::inUpdateWindow(now) && backoff < normal) ? backoff : normal;
        }
    } else {
        sleepSec = power::secondsUntilNextUpdate(now);
    }
    Serial.printf("[main] awake %lu ms\n", (unsigned long)millis());
    power::sleepFor(sleepSec);
}

void loop() {
    // Not reached: setup() always ends in deep sleep.
}
