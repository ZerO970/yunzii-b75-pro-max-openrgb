# YUNZII B75 PRO MAX — OpenRGB **+ Screen** firmware (QMK)

![YUNZII B75 PRO MAX running a music-reactive OpenRGB effect](docs/demo.gif)

*Per-key RGB control via OpenRGB Direct mode — here driving a live music visualizer.*

Custom **QMK firmware for the YUNZII B75 PRO MAX** that enables the **OpenRGB protocol** *and* keeps the **built-in LCD screen working at the same time** — in a single firmware. The keyboard is detected and controllable in [OpenRGB](https://openrgb.org/) (including **Direct mode** for per-key control: music visualizers, ambient/screen lighting, game integrations), while the little TFT on the top-right can still show album art, a system monitor, a clock, and more via the companion app **[Yunzii Display Studio](https://github.com/ZerO970/yunzii-display)**.

The **stock YUNZII firmware ships with VIA but not OpenRGB**, so OpenRGB fails to detect the board with:

```
[QMK OpenRGB] Detection failed - the detected keyboard does not have the
OpenRGB protocol feature enabled! ... add OPENRGB_ENABLE = yes ...
```

This firmware fixes that by porting the OpenRGB QMK protocol into YUNZII's own QMK source tree — and then goes further (see **[What's new in v2](#whats-new-in-v2)**).

> Keywords: YUNZII B75 PRO MAX OpenRGB, B75 Pro Max QMK firmware, YUNZII OpenRGB support, B75 Pro Max RGB control, YUNZII B75 screen software, OpenRGB detection failed YUNZII.

---

## What's new in v2

The first release enabled OpenRGB but dropped the screen and the onboard lighting hotkeys, and OpenRGB's mode list was mislabeled. **v2 fixes all of that in one firmware:**

- 🖥️ **The LCD screen works on this firmware now.** Previously the screen only worked on the stock VIA firmware. v2 routes the vendor's screen protocol and OpenRGB through the *same* raw-HID endpoint (they use non-overlapping command IDs), so **one firmware drives both** the screen and OpenRGB — no more choosing. Drive the screen with the companion app **[Yunzii Display Studio](https://github.com/ZerO970/yunzii-display)** (album art / Now Playing, CPU-RAM-temp monitor, clock, weather, pomodoro, …).
- 🌈 **Keyboard lighting hotkeys are back — and better.** All the `Fn`-layer RGB controls work again (mode, hue, saturation, brightness, speed — **both directions**), and the colour/brightness/speed keys now support **smooth hold-to-adjust**: hold the key and the value glides instead of needing 20 taps.
- 🎨 **Every onboard animation is enabled** (~40 effects, up from ~18), and the board **boots into a real animation** instead of a blank Direct-mode frame. Cycle them with `Fn` + `\`.
- ✅ **OpenRGB's onboard mode list is fixed.** The old build reported a hardcoded effect table that didn't match this fork (renamed/extra effects), so modes were mislabeled or missing. v2 enumerates the **actually-compiled** effects dynamically, so every mode OpenRGB shows is real and selectable.
- ⌨️ **3 programmable "screen" hotkeys.** `Fn` + `5` / `6` / `7` send a raw-HID event to the companion app; in the app you bind each to a screen mode (monitor, Now Playing, clock, …). They type nothing and do nothing if the app isn't running.

Full details in [`CHANGELOG.md`](CHANGELOG.md).

---

## Board details (verified)

| | |
|---|---|
| Model | YUNZII B75 PRO MAX (tri-mode BT / 2.4G / USB-C) |
| MCU | STM32F103xB (ARM Cortex-M3), 128 KB flash |
| USB VID / PID | `0x28E9` / `0x31C8` |
| Matrix | 6 rows × 15 cols |
| RGB | RGB Matrix, 85 LEDs, AW20216S SPI driver |
| Screen | 160×96 TFT, RGB565, on a separate MCU behind the STM32 over UART |
| Bootloader | stm32duino (Maple), shows as `1EAF:0003` |
| QMK base | 0.24.2 (vendor fork) |
| OpenRGB protocol | v0x0D |
| Firmware size | ~64 KB (fits the 128 KB flash with room to spare) |

---

## Download & flash

A prebuilt binary is in [`/bin`](bin/) → **`smart_kb16_mk25047_openrgb.bin`** (~64 KB).

**⚠️ Back up your stock firmware first if you can, and keep the stock VIA `.bin` — it's your recovery image.**

1. Install [QMK Toolbox](https://github.com/qmk/qmk_toolbox).
2. Load `smart_kb16_mk25047_openrgb.bin` (the "MCU (AVR only)" dropdown is irrelevant — STM32 is auto-detected).
3. Enter the bootloader: **unplug → hold `ESC` → plug in USB → release `ESC`.** The log should show `STM32DUINO device connected (1EAF:0003)` and the **Flash** button lights up. (If the bootloader isn't detected, install drivers via QMK Toolbox → Tools → Install Drivers.)
4. Click **Flash**.

The Flash button stays greyed out while the keyboard is in normal mode (`28E9:31C8`) — that's expected. It only activates in bootloader mode (`1EAF:...`).

### After flashing — one-time note about the default effect

The keyboard boots into a real animation now, but RGB Matrix settings live in EEPROM, and your unit may still hold the old **Direct-mode** state (a static frame) from the previous firmware. If the lighting looks "frozen" or ignores the colour hotkeys, just press **`Fn` + `\`** once or twice to step off Direct mode onto an animation — or clear EEPROM. After that the hotkeys behave normally.

---

## Keyboard RGB hotkeys (no software needed)

Hold **`Fn`** and press:

| Keys | Action |
|---|---|
| `Fn` + `\` / `Fn` + `]` | Next / previous animation |
| `Fn` + `Enter` / `Fn` + `\|` (ISO) | Hue + / − *(hold to glide)* |
| `Fn` + `'` / `Fn` + `;` | Saturation + / − *(hold to glide)* |
| `Fn` + `↑` / `Fn` + `↓` | Brightness + / − *(hold to glide)* |
| `Fn` + `←` / `Fn` + `→` | Effect speed − / + *(hold to glide)* |
| `Fn` + `Backspace` | Toggle lighting on / off |
| `Fn` + `5` / `6` / `7` | Screen-app hotkeys (see below) |

A single tap makes a small step; **holding** a colour/brightness/speed key sweeps it smoothly.

---

## Using it with OpenRGB

- Use the **USB-C cable**. OpenRGB talks over USB raw HID; it does **not** work over Bluetooth / 2.4G.
- The board **auto-switches into Direct mode** as soon as OpenRGB streams color data — so music visualizers / ambient tools "just work" without manually selecting a mode.
- For an audio visualizer or screen ambilight, the [OpenRGB Effects Plugin](https://openrgb.org/plugin_effects.html) (Audio Visualizer / Ambilight effects) or [KeyboardVisualizer](https://gitlab.com/CalcProgrammer1/KeyboardVisualizer) work well.

---

## Using the screen — companion app

The TFT is driven over USB by **[Yunzii Display Studio](https://github.com/ZerO970/yunzii-display)**, an open-source Windows companion app (Now Playing with album art, CPU/RAM/temperature monitor, clock, weather, pomodoro, rotation/auto modes, tray + autostart). It runs alongside OpenRGB — both share the raw-HID channel with non-overlapping commands. See that repo for downloads and details.

> The screen's own MCU is still a black box (a closed vendor blob spoken to over UART). We drive it through the reverse-engineered picture protocol, which works great but has a hard ~0.3–1 s/frame ceiling. **If you can help open up that module, it unlocks real-time sync — see the [Yunzii Display Studio](https://github.com/ZerO970/yunzii-display) "call for help".**

---

## Known limitations

- **No VIA on this firmware.** OpenRGB and VIA both use raw HID and cannot coexist (QMK enforces this at build time). Flash the stock VIA firmware back if you need the vendor configurator or the module OTA updater. *(An OpenRGB layout JSON and a VIA definition JSON for this board are provided in the [Yunzii Display Studio](https://github.com/ZerO970/yunzii-display) repo for reference.)*
- **USB only** for RGB control and the screen (wireless typing is unaffected).
- **Screen frame rate is hardware-limited** to ~0.3–1 s/frame by the module's UART — great for album art / stats / clocks, not for high-FPS video. This is the vendor module, not the firmware.
- Works on my unit; flashing custom firmware is at your own risk.

---

## Build from source

Requires a QMK build environment (e.g. [QMK MSYS](https://msys.qmk.fm/) on Windows, or `qmk setup` on Linux/macOS) with the ARM toolchain (`arm-none-eabi-gcc`).

```sh
qmk compile -kb smart_kb16/mk25047 -km openrgb
# output: .build/smart_kb16_mk25047_openrgb.bin
```

### What was changed vs. the stock source

New files:
- `quantum/openrgb.c` / `quantum/openrgb.h` — the OpenRGB raw-HID protocol handler. **v2:** `raw_hid_receive()` dispatches `data[0] >= 0x0a` to the vendor screen handler before the OpenRGB switch, and `openrgb_get_enabled_modes()` enumerates compiled effects dynamically.
- `quantum/rgb_matrix/animations/openrgb_direct_anim.h` — the `OPENRGB_DIRECT` custom RGB Matrix effect.
- `keyboards/smart_kb16/mk25047/keymaps/openrgb/` — a VIA-free keymap. **v2:** enables all RGB Matrix effects, sets a real default mode, and adds the full set of RGB hotkeys plus the three `Fn`+`5/6/7` screen hotkeys.

Core wiring (see [`src/patches/`](src/patches)):
- `builddefs/common_features.mk` — an `OPENRGB_ENABLE` block (adds `openrgb.c`, `-DOPENRGB_ENABLE`, forces `RAW_ENABLE`, errors if VIA is also on).
- `quantum/rgb_matrix/animations/rgb_matrix_effects.inc` — registers the `OPENRGB_DIRECT` effect.

Vendor-file edits for v2 (screen dispatch reuse, smooth RGB hold, screen hotkeys) are documented in [`src/patches/mk25047_v2_changes.md`](src/patches/mk25047_v2_changes.md). The screen relay reuses the vendor's own `via_command_kb()`, already present in the stock tree under `Screen_AP` (enabled in `common.h`).

Port fixes needed for this board / newer QMK:
- The `OPENRGB_DIRECT` effect's `#include "openrgb.h"` must sit inside the `RGB_MATRIX_CUSTOM_EFFECT_IMPLS` pass, or it lands inside the `enum rgb_matrix_effects` definition and breaks the build.
- `rgb_matrix_off_flag` / `rgblight_off_flag` are defined in the vendor's `via.c`; since VIA isn't compiled here, the `openrgb` keymap provides their storage.

---

## Recovery

The stm32duino bootloader is untouched. If anything goes wrong, re-enter the bootloader (**hold `ESC` + plug USB**) and flash the stock YUNZII `.bin` back. It is very hard to brick.

---

## FAQ

**Can I use OpenRGB and the screen at the same time?**
Yes — that's the whole point of v2. Run OpenRGB and [Yunzii Display Studio](https://github.com/ZerO970/yunzii-display) together; they share the keyboard's raw-HID channel with non-overlapping commands.

**Does it work with SignalRGB?**
Not directly. SignalRGB doesn't speak the OpenRGB protocol and has no native plugin for the B75 PRO MAX. You *can* route SignalRGB's effects through the [SignalRGB → OpenRGB bridge](https://github.com/Fefedu973/SignalRGB-To-OpenRGB-Bridge). Important: **don't run OpenRGB and SignalRGB at the same time** — they both try to own the raw-HID channel and will fight (flicker/glitches).

**Does OpenRGB control work over Bluetooth / 2.4 GHz?**
No — OpenRGB and the screen are USB-only here. Use the USB-C cable (typing still works wirelessly).

**Can I still use VIA or the vendor configurator?**
Not on this firmware — OpenRGB and VIA can't coexist. Flash the stock firmware back if you need VIA.

**Will this brick my keyboard?**
Very unlikely. The stm32duino bootloader is untouched — hold `ESC` while plugging in USB, then flash the stock `.bin` to revert.

---

## Credits

- Firmware port, screen integration and RGB work by **Vitalii Zaikin (ZaviNi)** — [@ZerO970](https://github.com/ZerO970).
- [OpenRGB](https://openrgb.org/) by Adam Honse (CalcProgrammer1).
- OpenRGB QMK protocol by **Kasper24** ([original QMK PR #10961](https://github.com/qmk/qmk_firmware/pull/10961)) and the newer-QMK port by **[Domi2803/qmk-openrgb](https://github.com/Domi2803/qmk-openrgb)**.
- YUNZII / SmartCloud for the original board QMK source.

## Disclaimer

Not affiliated with or endorsed by YUNZII. Provided as-is, no warranty. Flashing custom firmware is at your own risk. Licensed **GPLv2+** (inherited from QMK).
