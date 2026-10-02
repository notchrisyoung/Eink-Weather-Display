#pragma once
// ---------------------------------------------------------------------------
// User settings. Wi-Fi and the API key live in secrets.h (not committed) -
// copy secrets.example.h to secrets.h and fill it in.
// ---------------------------------------------------------------------------

// ---- Where to forecast ----------------------------------------------------
#define WX_LATITUDE    "33.45"        // decimal degrees
#define WX_LONGITUDE   "-112.07"
#define WX_PLACE_NAME  "Home"         // shown in the header
#define WX_LANGUAGE    "en"           // OpenWeatherMap description language
#define WX_IMPERIAL    1              // 1 = °F, mph, inHg   0 = °C, km/h, hPa

// POSIX time-zone string, e.g. "PST8PDT,M3.2.0,M11.1.0" or "MST7" (Arizona).
#define WX_TIMEZONE    "MST7"
#define CLOCK_24H      0

// ---- Update schedule -------------------------------------------------------
#define UPDATE_EVERY_MIN   15          // wakes on the :00 / :15 / :30 / :45
#define QUIET_FROM_MIN     (23 * 60 + 30)  // no updates from 23:30 ...
#define QUIET_UNTIL_MIN    (6 * 60)        // ... until 06:00
#define RETRY_AFTER_FAIL_MIN 5         // first retry after a failed update

// ---- Hardware --------------------------------------------------------------
#define PIN_BATTERY_ADC    36          // LilyGo T5 4.7" battery sense (1:2 divider)
#define BATTERY_DIVIDER    2.0f
#define PIN_TEMP_PROBE     15          // DS18B20 data line (4.7k pull-up to 3V3)
#define TEMP_PROBE_LABEL   "Outside"   // label for the probe reading
#define SOUTHERN_HEMISPHERE 0          // flips the moon drawing
