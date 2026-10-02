#pragma once
#include <stdint.h>

namespace net {
// Join Wi-Fi, giving up after `timeoutMs`. Returns true when connected.
bool connect(const char *ssid, const char *password, uint32_t timeoutMs);
// Signal strength of the current connection in dBm (0 if not connected).
int rssi();
// Set the RTC from NTP and apply the POSIX time zone. Returns true on success.
bool syncClock(const char *posixTz, uint32_t timeoutMs);
// Apply the time zone without NTP (the RTC keeps running through deep sleep).
void useTimeZone(const char *posixTz);
// Radio off - saves power before the slow e-paper refresh.
void shutdown();
}
