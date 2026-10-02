#pragma once
#include "weather.h"

namespace owm {
// Download and decode the One Call 3.0 forecast for the configured location.
// Returns false (leaving `out` untouched) on any network or parse error.
bool fetch(Weather &out);
}
