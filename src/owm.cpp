// OpenWeatherMap One Call API 3.0 client.
// Docs: https://openweathermap.org/api/one-call-3
#include "owm.h"
#include "config.h"
#include "credentials.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

namespace owm {

static String requestUrl() {
    String url = "https://api.openweathermap.org/data/3.0/onecall?lat=";
    url += WX_LATITUDE;
    url += "&lon=";
    url += WX_LONGITUDE;
    url += "&exclude=minutely,alerts&units=";
    url += WX_IMPERIAL ? "imperial" : "metric";
    url += "&lang=";
    url += WX_LANGUAGE;
    url += "&appid=";
    url += OWM_API_KEY;
    return url;
}

// Only these fields are kept while streaming the (~30 KB) response, so the
// parsed document stays small.
static void buildFilter(JsonDocument &f) {
    JsonObject cur = f["current"].to<JsonObject>();
    for (const char *k : { "dt", "temp", "feels_like", "humidity", "pressure", "uvi",
                           "wind_speed", "wind_gust", "wind_deg", "sunrise", "sunset" })
        cur[k] = true;
    cur["weather"][0]["id"] = true;
    cur["weather"][0]["description"] = true;
    cur["weather"][0]["icon"] = true;

    JsonObject hour = f["hourly"][0].to<JsonObject>();
    hour["dt"] = true;
    hour["temp"] = true;
    hour["pop"] = true;

    JsonObject day = f["daily"][0].to<JsonObject>();
    day["dt"] = true;
    day["temp"]["min"] = true;
    day["temp"]["max"] = true;
    day["pop"] = true;
    day["moon_phase"] = true;
    day["weather"][0]["id"] = true;
}

static void decode(const JsonDocument &doc, Weather &w) {
    JsonObjectConst cur = doc["current"];
    w.observedAt = cur["dt"].as<long>();
    w.temp       = cur["temp"] | NAN;
    w.feelsLike  = cur["feels_like"] | NAN;
    w.humidity   = cur["humidity"] | 0.0f;
    w.pressure   = cur["pressure"] | 0.0f;
    w.uvIndex    = cur["uvi"] | 0.0f;
    w.windSpeed  = cur["wind_speed"] | 0.0f;
    w.windGust   = cur["wind_gust"] | 0.0f;   // only present when gusty
    w.windFrom   = cur["wind_deg"] | 0.0f;
    w.sunrise    = cur["sunrise"].as<long>();
    w.sunset     = cur["sunset"].as<long>();

    JsonObjectConst now = cur["weather"][0];
    w.sky = skyFromCode(now["id"] | 800);
    const char *icon = now["icon"] | "01d";
    w.isNight = icon[0] && icon[strlen(icon) - 1] == 'n';
    strlcpy(w.condition, now["description"] | "", sizeof w.condition);

    JsonArrayConst hourly = doc["hourly"];
    w.hourCount = 0;
    for (JsonObjectConst h : hourly) {
        if (w.hourCount >= Weather::kHours) break;
        HourPoint &p = w.hours[w.hourCount++];
        p.at = h["dt"].as<long>();
        p.temp = h["temp"] | NAN;
        p.rainChance = h["pop"] | 0.0f;
    }

    JsonArrayConst daily = doc["daily"];
    JsonObjectConst today = daily[0];
    w.todayLow  = today["temp"]["min"] | NAN;
    w.todayHigh = today["temp"]["max"] | NAN;
    w.moonPhase = today["moon_phase"] | 0.0f;

    w.dayCount = 0;
    for (size_t i = 1; i < daily.size() && w.dayCount < Weather::kDays; i++) {
        JsonObjectConst d = daily[i];
        DayOutlook &o = w.days[w.dayCount++];
        o.at = d["dt"].as<long>();
        o.low = d["temp"]["min"] | NAN;
        o.high = d["temp"]["max"] | NAN;
        o.rainChance = d["pop"] | 0.0f;
        o.sky = skyFromCode(d["weather"][0]["id"] | 800);
    }
}

bool fetch(Weather &out) {
    WiFiClientSecure tls;
    tls.setInsecure();   // no CA bundle on the device; the request carries no secrets beyond the API key
    HTTPClient http;
    http.useHTTP10(true);   // no chunked encoding, so the body can be streamed into the parser
    http.setTimeout(15000);
    if (!http.begin(tls, requestUrl())) {
        Serial.println("[owm] could not start request");
        return false;
    }
    int status = http.GET();
    if (status != HTTP_CODE_OK) {
        Serial.printf("[owm] HTTP %d %s\n", status, status < 0 ? http.errorToString(status).c_str() : "");
        if (status == 401) Serial.println("[owm] check the API key and that One Call 3.0 is enabled");
        http.end();
        return false;
    }

    JsonDocument filter;
    buildFilter(filter);
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
    http.end();
    if (err) {
        Serial.printf("[owm] JSON error: %s\n", err.c_str());
        return false;
    }
    if (!doc["current"].is<JsonObject>()) {
        Serial.println("[owm] response had no current conditions");
        return false;
    }

    Weather w = {};
    decode(doc, w);
    out = w;
    Serial.printf("[owm] %.1f, %s, %u hours, %u days\n", w.temp, w.condition, w.hourCount, w.dayCount);
    return true;
}

}  // namespace owm
