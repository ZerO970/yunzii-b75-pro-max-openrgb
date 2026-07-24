# Changelog

## v2 — screen + OpenRGB in one firmware

The v1 build enabled OpenRGB but disabled the LCD screen and the onboard lighting
hotkeys, and OpenRGB's onboard-mode list didn't match the firmware. v2 fixes all of
it — **one firmware now runs the screen and OpenRGB together**, with full keyboard
lighting control back.

### Added
- **Screen support on the OpenRGB firmware.** `raw_hid_receive()` in `quantum/openrgb.c`
  now dispatches any report with `data[0] >= 0x0a` to the vendor's screen handler
  (`via_command_kb`) before the OpenRGB command switch. OpenRGB commands (`1..9`) and
  the screen/module commands (`0x0a`, `0x40`, `0x41`, `0x42`, `0x81`) don't overlap,
  so a single raw-HID endpoint serves both. Drive the screen with
  [Yunzii Display Studio](https://github.com/ZerO970/yunzii-display).
- **All ~40 RGB Matrix effects enabled** (was ~18) via the `openrgb` keymap `config.h`.
- **Smooth hold-to-adjust** for hue / saturation / brightness / speed hotkeys: a single
  tap makes a small step, holding auto-repeats it (~30 ms cadence after a 300 ms delay)
  so the value glides. The final value is written to EEPROM once, on release.
- **Full RGB hotkey set** on the `Fn` layer: mode next/prev, hue ±, saturation ±,
  brightness ±, speed ±, on/off. (v1 only had a partial one-direction set.)
- **Three programmable screen hotkeys** `Fn`+`5/6/7` — each sends a raw-HID event
  (`cmd 0x30`, `idx 1..3`) to the companion app, which binds it to a screen mode. They
  type nothing and are inert if the app isn't running.

### Changed
- **Default RGB mode** is now a real animation (`RGB_MATRIX_CYCLE_LEFT_RIGHT`) instead of
  `OPENRGB_DIRECT`, so the board lights up with an effect on boot and the colour hotkeys
  work immediately. OpenRGB still auto-switches to Direct mode when it streams data.
  *(Applies on EEPROM reset; existing units may keep their stored mode — press `Fn`+`\`
  to step off Direct, or clear EEPROM.)*
- **`openrgb_get_enabled_modes()` enumerates compiled effects dynamically** (`1..RGB_MATRIX_EFFECT_MAX-1`)
  instead of a hardcoded index table that assumed the canonical effect set. This fork
  renames/adds effects, so the old table drifted and OpenRGB showed mislabeled/missing
  modes. Now every reported mode is real and selectable.

### Fixed
- OpenRGB onboard-mode list mismatch (see above).
- Hardened the screen path: the host-supplied payload length in `via_command_kb` is
  clamped before `sdWrite`, preventing an out-of-bounds read into the LCD UART on a
  malformed packet.

### Notes
- Firmware size ~64 KB (was ~60 KB); still comfortably within the 128 KB flash.
- No bootloader changes; recovery is unchanged (hold `ESC` + plug USB, flash stock `.bin`).

## v1 — initial OpenRGB port

- Ported the OpenRGB QMK raw-HID protocol into the YUNZII vendor QMK tree in a VIA-free
  `openrgb` keymap, so OpenRGB detects and controls the board (per-key Direct mode).
- Prebuilt `.bin` (~60 KB) + build instructions.
