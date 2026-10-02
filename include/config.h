#pragma once
// ---------------------------------------------------------------------------
// User settings. Wi-Fi, the API key and your location live in secrets.h
// (not committed) - copy secrets.example.h to secrets.h and fill it in.
// ---------------------------------------------------------------------------

#include "credentials.h"   // secrets.h may override the location settings below

// ---- Where to forecast ----------------------------------------------------
// Defaults only - put your real coordinates in secrets.h so they stay private.
#ifndef WX_LATITUDE
#define WX_LATITUDE    "33.45"        // decimal degrees
#endif
#ifndef WX_LONGITUDE
#define WX_LONGITUDE   "-112.07"
#endif
#ifndef WX_PLACE_NAME
#define WX_PLACE_NAME  "Home"         // shown in the header
#endif
#define WX_LANGUAGE    "en"           // OpenWeatherMap description language
#define WX_IMPERIAL    1              // 1 = °F, mph, inHg   0 = °C, km/h, hPa

// POSIX time-zone string, e.g. "PST8PDT,M3.2.0,M11.1.0" or "MST7" (Arizona).
#ifndef WX_TIMEZONE
#define WX_TIMEZONE    "MST7"
#endif
#define CLOCK_24H      0

// ---- Update schedule -------------------------------------------------------
// Each window is {first update, last update, every N minutes}, times as HHMM
// local time. Outside every window the display sleeps (and keeps showing the
// last forecast). Default: every 15 min 06:00-16:45, every 30 min 17:00-22:00.
#define UPDATE_WINDOWS { {600, 1645, 15}, {1700, 2200, 30} }
#define RETRY_AFTER_FAIL_MIN 5         // first retry after a failed update

// ---- Wi-Fi speed-ups (optional) -------------------------------------------
// A fixed IP skips DHCP and saves a few hundred ms per wake. Pick an address
// outside your router's DHCP range, then uncomment all four lines.
// #define WIFI_STATIC_IP "192.168.1.60"
// #define WIFI_GATEWAY   "192.168.1.1"
// #define WIFI_SUBNET    "255.255.255.0"
// #define WIFI_DNS       "192.168.1.1"

// ---- Hardware --------------------------------------------------------------
#define PIN_BATTERY_ADC    36          // LilyGo T5 4.7" battery sense (1:2 divider)
#define BATTERY_DIVIDER    2.0f
// Calibration: measure the battery with a multimeter, compare with the
// "[power] battery" line in the serial log, and set this to meter / logged.
#define BATTERY_CALIBRATION 1.000f
#define PIN_TEMP_PROBE     15          // DS18B20 data line (4.7k pull-up to 3V3)
#define TEMP_PROBE_LABEL   "Inside"    // the probe is mounted on the display
#define SOUTHERN_HEMISPHERE 0          // flips the moon drawing
