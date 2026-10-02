#pragma once
#include <stdint.h>
#include <time.h>

namespace power {
// Battery voltage in volts. On the T5 4.7" the sense divider is only powered
// while the panel supply is on, so call this between epd_poweron/off.
float batteryVolts();

// Seconds to sleep so the next wake lands on the next update slot, skipping
// the configured overnight quiet window.
uint32_t secondsUntilNextUpdate(time_t now);

// Deep sleep; never returns.
void sleepFor(uint32_t seconds);
}
