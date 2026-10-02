# E-Ink Weather Display

A battery-powered weather dashboard for the **LilyGo T5 4.7" e-paper board** (ESP32 + 960x540, 16-level greyscale). Every 15 minutes it wakes up, pulls the forecast from OpenWeatherMap, reads a local temperature probe, redraws the panel, and goes back into deep sleep. E-paper keeps its image with no power, so the display stays readable between updates while the board draws almost nothing.

![Dashboard preview](docs/preview.png)

*Rendered from the firmware's own drawing code with sample data. See [tools/preview](tools/preview).*

<!-- PHOTOS: add a photo of the finished display here, e.g. ![On the wall](docs/photo.jpg) -->

## What's on the screen

- **Header:** place name, date, time of the last update, Wi-Fi signal and battery level
- **Now:** a large condition icon, the temperature, "feels like", a description, and today's high and low
- **Detail tiles:**
  - Wind speed and direction on a compass, with gusts
  - Humidity
  - Pressure
  - Sunrise and sunset
  - Moon phase, drawn to match tonight's moon
  - Your local probe, or the UV index if no probe is fitted
- **Next 24 hours:** temperature curve over hourly chance-of-rain columns, with the warmest hour marked
- **Next 4 days:** icon, high, low and chance of rain

## Behaviour

- Updates on the clock at :00, :15, :30 and :45 (`UPDATE_EVERY_MIN`).
- **Quiet hours:** no updates from 23:30 to 06:00 (`QUIET_FROM_MIN` / `QUIET_UNTIL_MIN`), to save battery.
- **If an update fails** (Wi-Fi down, API error), the last good forecast is redrawn from RTC memory with an "Offline" note. It then retries after 5, 10, 20… minutes, and doesn't redraw again until it succeeds.
- Wi-Fi is switched off before the slow panel refresh.

## Hardware

| Part | Notes |
|---|---|
| LilyGo T5 4.7" (ESP32-WROVER, PSRAM) | The framebuffer lives in PSRAM |
| DS18B20 temperature probe (optional) | Data on **GPIO 15** with a 4.7k pull-up to 3V3. Without it, the tile shows UV index instead. |
| 1-cell LiPo | Read through the board's divider on **GPIO 36** |

## Setup

1. Install [PlatformIO](https://platformio.org/) (the VS Code extension is easiest).
2. Get an [OpenWeatherMap](https://openweathermap.org/api/one-call-3) API key and subscribe it to **One Call API 3.0**. The free tier includes 1,000 calls a day, and this display uses about 70.
3. Copy `include/secrets.example.h` to `include/secrets.h`, then fill in your Wi-Fi details and API key. `secrets.h` is gitignored.
4. Edit `include/config.h`:
   - Latitude, longitude and place name
   - Units (imperial or metric) and POSIX time zone
   - 12/24-hour clock, update interval and quiet hours
5. `pio run -t upload`, then `pio device monitor` to watch the log.

## Project layout

| Path | What it does |
|---|---|
| `src/main.cpp` | Wake → connect → fetch → draw → sleep, plus offline fallback |
| `src/owm.*` | One Call 3.0 request, streamed and filtered JSON parsing |
| `src/weather.h` | Forecast data model and condition-code mapping |
| `src/screen.*` | Dashboard layout |
| `src/icons.*` | Weather icons, moon, wind dial, status glyphs (all drawn in code) |
| `src/canvas.*` | Drawing layer over the e-paper framebuffer |
| `src/net.*` | Wi-Fi and NTP time |
| `src/power.*` | Battery voltage, sleep scheduling |
| `src/probe.*` | DS18B20 reading |
| `src/fonts/` | Generated Open Sans bitmap fonts |
| `tools/make_fonts.py` | Regenerates `src/fonts/` from the TTFs in `assets/fonts/` |
| `tools/preview/` | Renders the layout to a PNG on a PC |

## Credits

Inspired by the many ESP32 e-paper weather displays in the maker community. Uses the [LilyGo EPD47](https://github.com/Xinyuan-LilyGO/LilyGo-EPD47) driver, [ArduinoJson](https://arduinojson.org/), the OneWire and DallasTemperature libraries, and weather data from [OpenWeatherMap](https://openweathermap.org/). The fonts are generated from [Open Sans](https://fonts.google.com/specimen/Open+Sans), which is under the SIL Open Font License (`assets/fonts/OFL.txt`).
