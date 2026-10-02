#pragma once
#include "weather.h"

namespace owm {
// Download and decode the One Call 3.0 forecast for the configured location.
// Returns false (leaving `out` untouched) on any network or parse error.
// If `serverTime` is given it receives the server's UTC clock from the HTTP
// Date header (0 if unavailable), even when the forecast itself fails.
bool fetch(Weather &out, time_t *serverTime);
// Short human-readable reason the last fetch failed ("" after a success).
const char *lastError();
}
