#pragma once
#include "canvas.h"
#include "weather.h"

// Weather pictogram centred on (cx, cy), roughly `size` pixels wide.
void drawSkyIcon(Canvas &c, int cx, int cy, int size, Sky sky, bool night);

// Moon disc of radius r, shaded for `phase` (0 = new, 0.5 = full).
void drawMoon(Canvas &c, int cx, int cy, int r, float phase, bool southern);

// Small status glyphs for the header (drawn in `tone` on a dark bar).
void drawWifiBars(Canvas &c, int x, int baseline, int rssi, uint8_t tone, uint8_t off);
void drawBattery(Canvas &c, int x, int y, int percent, uint8_t tone);

// Compass dial with an arrow pointing where the wind is blowing *to*.
void drawWindDial(Canvas &c, int cx, int cy, int r, float fromDegrees);

// Tiny raindrop, used next to precipitation chances.
void drawDrop(Canvas &c, int cx, int cy, int h, uint8_t tone);
