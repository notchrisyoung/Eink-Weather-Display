#include "net.h"
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

namespace net {

bool connect(const char *ssid, const char *password, uint32_t timeoutMs) {
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(ssid, password);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > timeoutMs) {
            Serial.printf("[net] no Wi-Fi after %lu ms (status %d)\n", (unsigned long)timeoutMs, WiFi.status());
            return false;
        }
        delay(100);
    }
    Serial.printf("[net] connected %s, %d dBm\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
    return true;
}

int rssi() { return WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0; }

void useTimeZone(const char *posixTz) {
    setenv("TZ", posixTz, 1);
    tzset();
}

bool syncClock(const char *posixTz, uint32_t timeoutMs) {
    configTzTime(posixTz, "pool.ntp.org", "time.google.com", "time.cloudflare.com");
    struct tm tm;
    if (!getLocalTime(&tm, timeoutMs)) {
        Serial.println("[net] NTP sync failed");
        return false;
    }
    Serial.printf("[net] clock %04d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1,
                  tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    return true;
}

void shutdown() {
    WiFi.disconnect(true, false);
    WiFi.mode(WIFI_OFF);
}

}  // namespace net
