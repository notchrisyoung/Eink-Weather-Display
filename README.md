# E-Ink Weather Display

A battery-powered weather station display built on the **LilyGo T5 4.7" e-paper board** (ESP32 + 960x540 EPD). It wakes up every 15 minutes, pulls current conditions and a 3-hour-step forecast from OpenWeatherMap, draws everything to the e-ink panel, then goes back into deep sleep. Because e-paper holds its image with no power, the display stays readable between updates while the board draws almost nothing.

<!-- PHOTOS: add a photo of the finished display here, e.g. ![Weather display](docs/photo.jpg) -->

## What it shows

- City, date, time of last update, and a **local temperature reading** from a DS18B20 probe wired to the board
- Current temperature, conditions text and a large weather icon
- Wind speed and direction on a compass rose
- Sunrise/sunset times and the current moon phase (drawn moon icon)
- Forecast boxes for the next several 3-hour periods, with small icons
- Wi-Fi signal strength and battery level

## Hardware

| Part | Notes |
|---|---|
| LilyGo T5 4.7" EPD (ESP32, PSRAM) | Uses the [LilyGo-EPD47](https://github.com/Xinyuan-LilyGO/LilyGo-EPD47) driver |
| DS18B20 temperature sensor | Data on **GPIO 15** (`ONE_WIRE_BUS`), with the usual 4.7k pull-up |
| LiPo battery | Read on GPIO 36 by `BatteryVoltage.cpp` |

## Power behaviour

- Normal updates every **15 minutes** (`SleepDuration`), aligned to the clock (:00, :15, :30, :45).
- From **23:30 to 06:00** (`LongSleepStart` / `LongSleepEnd`) it skips updates entirely and sleeps through the night.
- Wi-Fi is switched off before the panel is drawn to save power.

## Setup

1. Install the Arduino IDE with the **ESP32 board package**, and select a board with PSRAM enabled (e.g. *ESP32 Dev Module*, PSRAM: Enabled).
2. Install the libraries:
   - [LilyGo-EPD47](https://github.com/Xinyuan-LilyGO/LilyGo-EPD47)
   - ArduinoJson
   - OneWire
   - DallasTemperature
3. Edit `owm_credentials.h`:
   - `ssid` / `password`: your Wi-Fi
   - `apikey`: a free key from [openweathermap.org](https://openweathermap.org/)
   - `Lat`, `Lon`, `City`, `Country`, `Units`, `Timezone`
4. Open `OWM_EPD47_epaper_v2.5.ino` and upload.

> **Heads-up:** the sketch requests OpenWeatherMap's `onecall` endpoint. OWM has retired One Call 2.5 for new keys, so a new key may need a (free-tier) **One Call 3.0** subscription, and the request URL updated to match.

> **Don't commit your real Wi-Fi password or API key.** Keep them only in your local copy of `owm_credentials.h`.

## Files

| File | Purpose |
|---|---|
| `OWM_EPD47_epaper_v2.5.ino` | Main sketch: Wi-Fi, weather fetch/decode, drawing, deep sleep |
| `owm_credentials.h` | Wi-Fi, API key, location and time-zone settings |
| `forecast_record.h` | Struct holding decoded weather data |
| `lang.h` | Display text strings |
| `moon.h` | Moon phase helpers |
| `BatteryVoltage.cpp/.h` | Battery voltage / charge-level reading |
| `opensans*.h` | Fonts used by the sketch |
| `Font Files/` | Larger set of pre-converted Open Sans fonts in other sizes |

## Credits

Based on David Bird's (G6EJD) ESP32 OpenWeatherMap e-paper weather display for the LilyGo 4.7" EPD. His original copyright notice is kept in the sketch header and applies to that code. This version is modified from his, for example with the DS18B20 local temperature reading in the header line.
