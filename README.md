<p align="right"><a href="README.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Retrograde Pocket Watch

A classical pocket watch that travels into the past. For every hour that passes in real life, the watch moves back one hour. All three hands run backwards, and the separate date display follows through midnight, month boundaries and previous years.

A brass case, ivory dial, Roman numerals and a continuously sweeping red seconds hand create a quiet desk companion for watching time retreat.

## Download and use

Download the complete firmware from [Releases](https://github.com/jonwong0882-source/reverse-pocket-watch-ai-passport/releases). Flashing the complete image replaces the current application and may erase existing settings. No Wi-Fi or account is required.

1. The watch starts reversing on boot. **UNSET** means its starting date needs calibration: the build date is only a suggestion, not verified current time.
2. Hold OK to set the starting time. UP/DOWN adjusts the value; press OK to advance through year, month, day, hour, minute and second.
3. The final OK starts a reverse timeline from the selected date/time. Hold OK during setup to cancel while the previous timeline continues.
4. On the watch screen, UP/DOWN selects 10/20/35/50/70/100 percent brightness.

Set the time again after each power cycle. This offline version cannot account for time spent powered off. The interface uses English labels and numeric dates.

## Preview and verification

The Release cover is a host rendering of the actual application code. Its date and battery values are sample data; **it is not a device photograph or device screenshot**.

- Build: PASS; merged image and matching debug artifacts verified.
- Host tests: PASS, including equal-rate reversal, leap/century years, date bounds, button setup, actual screen rendering and 1,000 consecutive frames.

## Build

Target hardware: FoloToy AI Passport. Activate ESP-IDF 5.5.3, then run:

```sh
./tools/validate.sh
```

The complete image is `build/FoloToy-AI-Passport-full.bin`, intended for offset `0x0`. See [development notes](docs/reverse-pocket-watch.md) for the implementation and host-rendering commands.

Based on [FoloToy AI Passport](https://gitee.com/FoloToy/ai-passport) under its MIT license; see [LICENSE](LICENSE).
