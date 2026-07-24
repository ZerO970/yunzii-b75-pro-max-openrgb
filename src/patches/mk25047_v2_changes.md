# v2 vendor-file edits — `keyboards/smart_kb16/mk25047/`

The screen-relay and RGB-hotkey features touch two **vendor** files that this repo does
not ship in full (they're large and part of YUNZII's tree). Apply these edits to your
vendor QMK fork before building. Line numbers are approximate; match on the surrounding
code. Everything else (the screen dispatch and dynamic mode list) lives in the shipped
`src/quantum/openrgb.c` and the `openrgb` keymap.

> Note: `via_command_kb()` and its screen branch already exist in the stock tree, guarded
> by `#ifdef Screen_AP` — and `Screen_AP` is `#define`d unconditionally in
> `keyboards/smart_kb16/mk25047/common.h`. So the screen handler is already compiled; v2
> only needed `openrgb.c` to *call* it (see `src/quantum/openrgb.c`). No change to the
> screen branch itself is required.

---

## 1. `mk25047.h` — three custom keycodes

Add to the custom keycode `enum` (after `Scr_Time`):

```c
    Scr_EN,
    Scr_Time,
    // Screen-app hotkeys: press sends a raw-HID event (cmd 0x30, idx 1..3) to the
    // host companion app. They type nothing; if the app isn't running, nothing happens.
    SCR_HK1,
    SCR_HK2,
    SCR_HK3
};
```

## 2. `mk25047.c` — smooth RGB hold + screen hotkeys

Just above `process_record_kb`, add the hold state, a small-step helper, and saturating
add/sub helpers (`qadd8`/`qsub8` aren't visible in this translation unit):

```c
#define RGB_HOLD_DELAY 300   // ms held before auto-repeat starts
#define RGB_HOLD_RATE  30    // ms between repeat steps
static uint16_t rgb_hold_kc    = 0;
static uint32_t rgb_hold_start = 0;
static uint32_t rgb_hold_timer = 0;

static bool is_rgb_adjust_kc(uint16_t kc) {
    return kc == RGB_HUI || kc == RGB_HUD || kc == RGB_SAI || kc == RGB_SAD ||
           kc == RGB_VAI || kc == RGB_VAD || kc == RGB_SPI || kc == RGB_SPD;
}

static inline uint8_t sat_add(uint8_t v, uint8_t d) { return (v > (uint8_t)(255 - d)) ? 255 : (uint8_t)(v + d); }
static inline uint8_t sat_sub(uint8_t v, uint8_t d) { return (v < d) ? 0 : (uint8_t)(v - d); }

static void rgb_apply_step(uint16_t kc) {
    HSV hsv = rgb_matrix_get_hsv();
    switch (kc) {
        case RGB_HUI: rgb_matrix_sethsv_noeeprom((uint8_t)(hsv.h + 3), hsv.s, hsv.v); break;
        case RGB_HUD: rgb_matrix_sethsv_noeeprom((uint8_t)(hsv.h - 3), hsv.s, hsv.v); break;
        case RGB_SAI: rgb_matrix_sethsv_noeeprom(hsv.h, sat_add(hsv.s, 4), hsv.v); break;
        case RGB_SAD: rgb_matrix_sethsv_noeeprom(hsv.h, sat_sub(hsv.s, 4), hsv.v); break;
        case RGB_VAI: rgb_matrix_sethsv_noeeprom(hsv.h, hsv.s, sat_add(hsv.v, 4)); break;
        case RGB_VAD: rgb_matrix_sethsv_noeeprom(hsv.h, hsv.s, sat_sub(hsv.v, 4)); break;
        case RGB_SPI: rgb_matrix_set_speed_noeeprom(sat_add(rgb_matrix_get_speed(), 4)); break;
        case RGB_SPD: rgb_matrix_set_speed_noeeprom(sat_sub(rgb_matrix_get_speed(), 4)); break;
        default: break;
    }
}
```

At the **top of `process_record_kb`**, intercept the screen hotkeys and the RGB-adjust keys:

```c
    /* Screen-app hotkeys -> raw-HID event to the companion app (no keystroke) */
    if (keycode == SCR_HK1 || keycode == SCR_HK2 || keycode == SCR_HK3) {
        if (record->event.pressed) {
            uint8_t report[64] = {0};                  // raw-HID endpoint size on this board
            report[0] = 0x30;                          // app-hotkey namespace (free vs OpenRGB 1..9, screen 0x0a/0x40-0x42/0x81)
            report[1] = (uint8_t)(keycode - SCR_HK1) + 1;
            raw_hid_send(report, sizeof(report));
        }
        return false;
    }

    /* Smooth hold-to-adjust for RGB colour/brightness/speed keys */
    if (is_rgb_adjust_kc(keycode)) {
        if (rgb_matrix_off_flag || (batt_charge_status == batt_no_charge && temp <= 10)) return false;
        if (record->event.pressed) {
            rgb_apply_step(keycode);                   // immediate first step
            rgb_hold_kc = keycode; rgb_hold_start = timer_read32(); rgb_hold_timer = timer_read32();
        } else if (rgb_hold_kc == keycode) {
            rgb_hold_kc = 0;
            eeconfig_update_rgb_matrix();              // persist final value once
        }
        return false;
    }
```

In **`matrix_scan_kb`** (top of the function body), auto-repeat while held:

```c
    if (rgb_hold_kc && timer_elapsed32(rgb_hold_start) > RGB_HOLD_DELAY &&
        timer_elapsed32(rgb_hold_timer) > RGB_HOLD_RATE) {
        rgb_hold_timer = timer_read32();
        rgb_apply_step(rgb_hold_kc);
    }
```

## 3. `mk25047.c` — harden the screen payload length (defensive)

In `via_command_kb`, right after `data_len = ap_message_buff[3];`, clamp it so a malformed
host packet can't make `sdWrite(&SD3, ap_message_buff+7, data_len)` read past the 64-byte
buffer:

```c
    if (data_len > sizeof(ap_message_buff) - 7) {
        data_len = sizeof(ap_message_buff) - 7;
    }
```

## 4. Keymap

The `openrgb` keymap (shipped in `src/`) places `RGB_RMOD/HUD/SAI/SAD` and `SCR_HK1/2/3`
on the `Fn` layers — see `src/keyboards/smart_kb16/mk25047/keymaps/openrgb/keymap.c`.
