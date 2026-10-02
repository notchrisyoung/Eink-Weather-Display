#pragma once
#include "canvas.h"
#include "weather.h"

// Full-screen dashboard: header, current conditions, detail tiles,
// 24-hour chart and the next four days.
void drawWeatherScreen(Canvas &c, const Weather &w, const DeviceStatus &st);

// Plain fallback when there is no forecast to show yet (first boot offline).
void drawMessageScreen(Canvas &c, const char *title, const char *detail);

// LiPo voltage -> rough state of charge, 0..100 (or -1 if unknown).
int batteryPercent(float volts);
