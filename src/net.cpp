#include "net.h"
#include "config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <sys/time.h>

namespace net {

// Remembered across deep sleep so the next wake can skip the channel scan.
RTC_DATA_ATTR static uint8_t savedBssid[6];
RTC_DATA_ATTR static int32_t savedChannel = 0;

static bool waitFor(uint32_t timeoutMs) {
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > timeoutMs) return false;
        delay(20);
    }
    return true;
}

bool connect(const char *ssid, const char *password, uint32_t timeoutMs) {
    uint32_t start = millis();
    WiFi.persistent(false);   // don't rewrite flash on every wake
    WiFi.mode(WIFI_STA);
#ifdef WIFI_STATIC_IP
    {
        IPAddress ip, gw, mask, dns;
        ip.fromString(WIFI_STATIC_IP);
        gw.fromString(WIFI_GATEWAY);
        mask.fromString(WIFI_SUBNET);
        dns.fromString(WIFI_DNS);
        WiFi.config(ip, gw, mask, dns);   // skips DHCP
    }
#endif
    bool ok = false;
    if (savedChannel > 0) {
        WiFi.begin(ssid, password, savedChannel, savedBssid, true);
        ok = waitFor(3000);
        if (!ok) {
            Serial.println("[net] fast reconnect failed, scanning");
            WiFi.disconnect(true, false);
            savedChannel = 0;
        }
    }
    if (!ok) {
        WiFi.begin(ssid, password);
        uint32_t used = millis() - start;
        ok = waitFor(timeoutMs > used ? timeoutMs - used : 0);
    }
    if (!ok) {
        Serial.printf("[net] no Wi-Fi (status %d)\n", WiFi.status());
        return false;
    }
    memcpy(savedBssid, WiFi.BSSID(), 6);
    savedChannel = WiFi.channel();
    Serial.printf("[net] connected in %lu ms, ch %ld, %d dBm\n", (unsigned long)(millis() - start),
                  (long)savedChannel, WiFi.RSSI());
    return true;
}

int rssi() { return WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0; }

void useTimeZone(const char *posixTz) {
    setenv("TZ", posixTz, 1);
    tzset();
}

void setClock(time_t utc) {
    struct timeval tv = { utc, 0 };
    settimeofday(&tv, nullptr);
}

bool syncClock(uint32_t timeoutMs) {
    configTzTime(WX_TIMEZONE, "pool.ntp.org", "time.google.com", "time.cloudflare.com");
    struct tm tm;
    if (!getLocalTime(&tm, timeoutMs)) {
        Serial.println("[net] NTP sync failed");
        return false;
    }
    Serial.println("[net] clock set from NTP");
    return true;
}

void shutdown() {
    WiFi.disconnect(true, false);
    WiFi.mode(WIFI_OFF);
}

}  // namespace net
