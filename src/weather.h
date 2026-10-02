#pragma once
// Plain data describing one forecast snapshot. Kept free of Arduino types so
// the screen code can also be built on a PC for previews (tools/preview).
#include <stdint.h>
#include <time.h>

enum class Sky : uint8_t {
    Clear, FewClouds, Clouds, Overcast, Drizzle, Rain, Storm, Snow, Sleet, Fog
};

struct HourPoint {
    time_t at;
    float  temp;
    float  rainChance;   // 0..1
};

struct DayOutlook {
    time_t at;
    float  low, high;
    float  rainChance;   // 0..1
    Sky    sky;
};

struct Weather {
    time_t observedAt;
    float  temp, feelsLike, humidity, pressure, uvIndex;
    float  windSpeed, windGust, windFrom;   // windFrom in degrees, 0 = north
    Sky    sky;
    bool   isNight;
    char   condition[40];   // e.g. "scattered clouds"

    float  todayLow, todayHigh;
    time_t sunrise, sunset;
    float  moonPhase;       // 0 = new, 0.25 first quarter, 0.5 full, 0.75 last quarter

    static const int kHours = 24;
    HourPoint hours[kHours];
    uint8_t   hourCount;

    static const int kDays = 4;
    DayOutlook days[kDays];  // starts with tomorrow
    uint8_t    dayCount;
};

// What the header/status area needs besides the forecast itself.
struct DeviceStatus {
    time_t now;
    bool   fresh;          // false = showing the last good forecast
    int    wifiRssi;       // dBm, 0 if not connected
    float  batteryVolts;   // < 1 if unknown
    float  probeTemp;      // NAN if no probe
};

// Map an OpenWeatherMap condition code (https://openweathermap.org/weather-conditions)
inline Sky skyFromCode(int code) {
    if (code >= 200 && code < 300) return Sky::Storm;
    if (code >= 300 && code < 400) return Sky::Drizzle;
    if (code == 511 || (code >= 611 && code <= 616)) return Sky::Sleet;
    if (code >= 500 && code < 600) return Sky::Rain;
    if (code >= 600 && code < 700) return Sky::Snow;
    if (code >= 700 && code < 800) return Sky::Fog;
    if (code == 800) return Sky::Clear;
    if (code == 801 || code == 802) return Sky::FewClouds;
    if (code == 803) return Sky::Clouds;
    return Sky::Overcast;
}
