<p align="right"><a href="reverse-pocket-watch.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Retrograde pocket watch

An offline application for the AI Passport 240 × 320 display. The brass case,
ivory dial, original vector Roman numerals (including traditional IIII), dark
hour/minute hands and red sweeping seconds hand form a dedicated watch screen.
The lower region shows the watch's Gregorian date and 24-hour time. The top
right shows battery state, or `--` when unavailable. No demo menu is reachable.

The watch reads `starting timestamp − elapsed monotonic microseconds`. One real
hour takes it one hour into the past. All three hands move backwards together;
a 40 ms timer targets 25 updates per second without accumulating timer drift.
Dates cross midnight, month/year boundaries, leap days and years before 1970.
The Gregorian calendar stops at year 0001 rather than overflowing.

## Starting time and controls

This offline board has no independent battery-backed real-time clock. On each
boot the watch begins reversing immediately, but its initial date is explicitly
**UNSET**: the build timestamp in UTC+8 is only a suggestion, not verified current
time. Set the actual starting date/time after each boot. No Wi-Fi, cloud service,
credentials or flash writes are needed. Settings are intentionally not persisted,
since a saved timestamp cannot account for time spent powered off.

- Hold OK to open starting-time setup. UP/DOWN changes the selected field.
- Press OK through year, month, day, hour, minute and second. The last OK starts
  a new reverse timeline at that selected date/time.
- Hold OK in setup to cancel. The previous timeline continues while editing.
- UP/DOWN on the watch changes brightness across 10/20/35/50/70/100 percent.

The UI uses English labels and numeric dates; the Roman numerals are original
vector strokes and need no external font. No Chinese glyph support is assumed.

## Validation and delivery

Use ESP-IDF 5.5.3 and `./tools/validate.sh`. The extra host test covers exact 1:1
and subsecond reversal, Gregorian leap/century boundaries, year bounds and field
editing. The default NVS/PHY/factory layout is unchanged. The independently
verified full image is `build/FoloToy-AI-Passport-full.bin`; matching ELF/MAP and
manifest are retained under its SHA-256 directory in `build/firmware/`.
A full image at offset 0 can reset NVS. Building does not authorize flashing.

For an actual LVGL host rendering and button-flow test, with managed components
already available:

```sh
cmake -S tests/watch_preview -B build/watch-preview
cmake --build build/watch-preview -j 8
cd build/watch-preview
./watch_preview
```

The executable compiles the same `main/watch_main.c` as the device, uses a 40 KiB
LVGL pool and writes PPM captures for the unset/main/settings screens and later
reverse-time frames. It tests setup/cancel/start and one-second/one-hour reversal.
This is a host rendering, not evidence of LCD quality, real frame rate, battery
readings or physical button behavior. Those checks require an approved flash.

## Validated build

Build: PASS. Host tests: PASS, including actual LVGL rendering and 1,000 consecutive frames. Device tests: NOT RUN. Unverified: physical LCD appearance, frame rate, battery readings and buttons. No USB modem port was detected during handoff.

Full image SHA-256: `da8685180921606e0c27761ddce9e5e5c046dbd1bba9fda14b6b1da01bab0042`

Matching ELF SHA-256: `f1fea5ca2c17f43663b396938116af50cef3b747cf514e0295c7dfb7442e9e7e`

Archive: `build/firmware/da8685180921606e0c27761ddce9e5e5c046dbd1bba9fda14b6b1da01bab0042`
