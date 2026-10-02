# Layout preview

Renders the dashboard on a PC using the firmware's own drawing code
(`src/screen.cpp`, `canvas.cpp`, `icons.cpp`) and made-up sample data, so you
can tweak the layout without flashing the board.

Linux / macOS / WSL:

```bash
pio pkg install            # once, downloads the EPD47 headers
pip install pillow
tools/preview/build.sh     # writes docs/preview.png
```

`host_epd.cpp` stands in for the few display-driver functions the screen code
calls (pixels and text). To see the other states, run the built binary
yourself, e.g. `tools/preview/out/preview out.pgm --offline --no-probe`.
