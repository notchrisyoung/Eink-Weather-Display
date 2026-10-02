#pragma once

namespace probe {
// Read the DS18B20 on PIN_TEMP_PROBE, in the display's units.
// Returns NAN when no sensor answers, so the screen can show something else.
float readTemperature();
}
