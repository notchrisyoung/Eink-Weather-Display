#pragma once
// Copy this file to secrets.h (same folder) and fill in your own values.
// secrets.h is listed in .gitignore so it never gets committed.

#define WIFI_SSID     "your-network"
#define WIFI_PASSWORD "your-password"

// OpenWeatherMap API key with a "One Call API 3.0" subscription
// (the free tier allows 1,000 calls/day; this display uses about 55).
#define OWM_API_KEY   "your-api-key"

// Optional: your location. Kept here rather than in config.h so exact
// coordinates never end up in the repo.
// #define WX_LATITUDE   "37.77"
// #define WX_LONGITUDE  "-122.42"
// #define WX_PLACE_NAME "Home"
// #define WX_TIMEZONE   "PST8PDT,M3.2.0,M11.1.0"
