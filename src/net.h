#pragma once
#include <stdint.h>
#include <time.h>

namespace net {
// Join Wi-Fi. Tries a fast reconnect first using the access point's channel
// and BSSID remembered from last time; falls back to a full scan if that
// fails. Returns true when connected.
bool connect(const char *ssid, const char *password, uint32_t timeoutMs);
// Signal strength of the current connection in dBm (0 if not connected).
int rssi();
// Set the RTC from NTP. Only needed when no other time source is available.
bool syncClock(uint32_t timeoutMs);
// Set the RTC from a UTC timestamp (e.g. an HTTP Date header).
void setClock(time_t utc);
// Apply the POSIX time zone for local-time conversions.
void useTimeZone(const char *posixTz);
// Radio off - saves power before the slow e-paper refresh.
void shutdown();
}
