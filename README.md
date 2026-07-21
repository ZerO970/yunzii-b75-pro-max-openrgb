# YUNZII B75 PRO MAX — OpenRGB firmware (QMK)

![YUNZII B75 PRO MAX running a music-reactive OpenRGB effect](docs/demo.gif)

*Per-key RGB control via OpenRGB Direct mode — here driving a live music visualizer.*

Custom **QMK firmware for the YUNZII B75 PRO MAX** that enables the **OpenRGB protocol**, so the keyboard is detected and controllable in [OpenRGB](https://openrgb.org/) — including **Direct mode** for per-key control (music visualizers, ambient/screen lighting, game integrations, etc.).

The **stock YUNZII firmware ships with VIA but not OpenRGB**, so OpenRGB fails to detect the board with:

```
[QMK OpenRGB] Detection failed - the detected keyboard does not have the
OpenRGB protocol feature enabled! ... add OPENRGB_ENABLE = yes ...
```

This firmware fixes that by porting the OpenRGB QMK protocol into YUNZII's own QMK source tree.

> Keywords: YUNZII B75 PRO MAX OpenRGB, B75 Pro Max QMK firmware, YUNZII OpenRGB support, B75 Pro Max RGB control, OpenRGB detection failed YUNZII.

---

## Board details (verified)

| | |
|---|---|
| Model | YUNZII B75 PRO MAX (tri-mode BT / 2.4G / USB-C) |
| MCU | STM32F103xB (ARM Cortex-M3), 128 KB flash |
| USB VID / PID | `0x28E9` / `0x31C8` |
| Matrix | 6 rows × 15 cols |
| RGB | RGB Matrix, 85 LEDs, AW20216S SPI driver |
| Bootloader | stm32duino (Maple), shows as `1EAF:0003` |
| QMK base | 0.24.2 (vendor fork) |
| OpenRGB protocol | v0x0D |

---

## Download & flash

A prebuilt binary is in [`/bin`](bin/) → **`smart_kb16_mk25047_openrgb.bin`** (~60 KB).

**⚠️ Back up your stock firmware first if you can, and keep the stock VIA `.bin` — this is your recovery image.**

1. Install [QMK Toolbox](https://github.com/qmk/qmk_toolbox).
2. Load `smart_kb16_mk25047_openrgb.bin` (the "MCU (AVR only)" dropdown is irrelevant — STM32 is auto-detected).
3. Enter the bootloader: **unplug → hold `ESC` → plug in USB → release `ESC`.** The log should show `STM32DUINO device connected (1EAF:0003)` and the **Flash** button lights up. (If the bootloader isn't detected, install drivers via QMK Toolbox → Tools → Install Drivers.)
4. Click **Flash**.

The Flash button stays greyed out while the keyboard is in normal mode (`28E9:31C8`) — that's expected. It only activates in bootloader mode (`1EAF:...`).

---

## Using it with OpenRGB

- Use the **USB-C cable**. OpenRGB talks over USB raw HID; it does **not** work over Bluetooth / 2.4G.
- The board boots into an **OpenRGB Direct** RGB Matrix effect and **auto-switches into Direct mode** as soon as OpenRGB streams color data — so music visualizers / ambient tools "just work" without manually selecting a mode.
- For an audio visualizer or screen ambilight, the [OpenRGB Effects Plugin](https://openrgb.org/plugin_effects.html) (Audio Visualizer / Ambilight effects) or [KeyboardVisualizer](https://gitlab.com/CalcProgrammer1/KeyboardVisualizer) work well.

---

## Known limitations

- **No VIA on this firmware.** OpenRGB and VIA both use raw HID with overlapping command IDs and cannot coexist (QMK enforces this at build time). While running this firmware, the **vendor configurator and the module/screen OTA update feature are unavailable** — flash the stock VIA firmware back if you need them.
- **USB only** for RGB control (see above).
- **Onboard hardware effect list is partial** in OpenRGB. The vendor reordered/added effects relative to stock QMK, so OpenRGB's hardcoded effect index table doesn't line up 1:1 and some onboard modes are mislabeled or missing. **Direct mode (the main use case) is unaffected.**
- Not extensively tested across every feature. Works on my unit; your mileage may vary.

---

## Build from source

Requires a QMK build environment (e.g. [QMK MSYS](https://msys.qmk.fm/) on Windows, or `qmk setup` on Linux/macOS) with the ARM toolchain (`arm-none-eabi-gcc`).

```sh
qmk compile -kb smart_kb16/mk25047 -km openrgb
# output: .build/smart_kb16_mk25047_openrgb.bin
```

### What was changed vs. the stock source

New files:
- `quantum/openrgb.c` / `quantum/openrgb.h` — the OpenRGB raw-HID protocol handler.
- `quantum/rgb_matrix/animations/openrgb_direct_anim.h` — the `OPENRGB_DIRECT` custom RGB Matrix effect.
- `keyboards/smart_kb16/mk25047/keymaps/openrgb/` — a VIA-free keymap with `OPENRGB_ENABLE = yes`.

Core wiring:
- `builddefs/common_features.mk` — an `OPENRGB_ENABLE` block (adds `openrgb.c`, `-DOPENRGB_ENABLE`, forces `RAW_ENABLE`, and errors if VIA is also on).
- `quantum/rgb_matrix/animations/rgb_matrix_effects.inc` — registers the `OPENRGB_DIRECT` effect.

Port fixes needed for this board / newer QMK:
- The `OPENRGB_DIRECT` effect's `#include "openrgb.h"` must sit inside the `RGB_MATRIX_CUSTOM_EFFECT_IMPLS` pass, or it lands inside the `enum rgb_matrix_effects` definition and breaks the build.
- `rgb_matrix_off_flag` / `rgblight_off_flag` are defined in the vendor's `via.c`; since VIA isn't compiled here, the `openrgb` keymap provides their storage.
- The Direct handlers auto-select the `OPENRGB_DIRECT` effect on incoming color data, so OpenRGB's "Direct" button works even after switching onboard modes.

---

## Recovery

The stm32duino bootloader is untouched. If anything goes wrong, re-enter the bootloader (**hold `ESC` + plug USB**) and flash the stock YUNZII `.bin` back. It is very hard to brick.

---

## FAQ

**Does it work with SignalRGB?**
Not directly. SignalRGB doesn't speak the OpenRGB protocol and has no native plugin for the B75 PRO MAX. You *can* route SignalRGB's effects to the keyboard through the [SignalRGB → OpenRGB bridge](https://github.com/Fefedu973/SignalRGB-To-OpenRGB-Bridge), which exposes OpenRGB devices to SignalRGB. Important: **don't run OpenRGB and SignalRGB at the same time** — they both try to own the keyboard's raw-HID channel and will fight each other, which shows up as flicker/glitches. This firmware is OpenRGB-native, so OpenRGB is the smoothest path.

**Does OpenRGB control work over Bluetooth / 2.4 GHz?**
No — OpenRGB is USB-only here. Use the USB-C cable for lighting control (typing still works wirelessly).

**Can I still use VIA or the vendor configurator?**
Not on this firmware — OpenRGB and VIA can't coexist. Flash the stock firmware back if you need VIA or the screen/BLE module OTA updater.

**Will this brick my keyboard?**
Very unlikely. The stm32duino bootloader is untouched — hold `ESC` while plugging in USB to enter it, then flash the stock `.bin` to revert.

---

## Credits

- [OpenRGB](https://openrgb.org/) by Adam Honse (CalcProgrammer1).
- OpenRGB QMK protocol by **Kasper24** ([original QMK PR #10961](https://github.com/qmk/qmk_firmware/pull/10961)) and the newer-QMK port by **[Domi2803/qmk-openrgb](https://github.com/Domi2803/qmk-openrgb)**.
- YUNZII / SmartCloud for the original board QMK source.

## Disclaimer

Not affiliated with or endorsed by YUNZII. Provided as-is, no warranty. Flashing custom firmware is at your own risk. Licensed **GPLv2+** (inherited from QMK).
