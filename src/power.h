#pragma once
#include <stdint.h>
#include <time.h>

namespace power {
// Resting battery voltage, smoothed across wakes. Call it first thing after
// waking, before Wi-Fi starts drawing current, and with the panel supply on
// (on the T5 4.7" the sense divider is fed from it).
float batteryVolts();

// True between the first and last update of any window (i.e. "daytime").
bool inUpdateWindow(time_t now);

// Seconds to sleep so the next wake lands on the next slot in UPDATE_WINDOWS.
uint32_t secondsUntilNextUpdate(time_t now);

// Deep sleep; never returns.
void sleepFor(uint32_t seconds);
}
