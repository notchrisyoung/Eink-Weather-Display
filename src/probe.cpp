#include "probe.h"
#include "config.h"
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

namespace probe {

float readTemperature() {
    OneWire bus(PIN_TEMP_PROBE);
    DallasTemperature sensors(&bus);
    sensors.begin();
    if (sensors.getDeviceCount() == 0) return NAN;
    sensors.setResolution(11);          // 0.125 degree steps, ~375 ms conversion
    sensors.requestTemperatures();      // blocks until the conversion is done
    float c = sensors.getTempCByIndex(0);
    if (c == DEVICE_DISCONNECTED_C || c < -55 || c > 125) return NAN;
    return WX_IMPERIAL ? c * 9.0f / 5.0f + 32.0f : c;
}

}  // namespace probe
