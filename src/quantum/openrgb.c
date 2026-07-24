/* Copyright 2020 Kasper
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/types.h>
#include "keymap_introspection.h"
#ifndef RAW_ENABLE
#    error "RAW_ENABLE is not enabled"
#endif

#include "version.h"
#include "quantum.h"
#include "openrgb.h"
#include "raw_hid.h"
#include "string.h"
#include <color.h>

#if !defined(OPENRGB_DIRECT_MODE_STARTUP_RED)
#    define OPENRGB_DIRECT_MODE_STARTUP_RED 0
#endif

#if !defined(OPENRGB_DIRECT_MODE_STARTUP_GREEN)
#    define OPENRGB_DIRECT_MODE_STARTUP_GREEN 0
#endif

#if !defined(OPENRGB_DIRECT_MODE_STARTUP_BLUE)
#    define OPENRGB_DIRECT_MODE_STARTUP_BLUE 255
#endif

RGB                  g_openrgb_direct_mode_colors[RGB_MATRIX_LED_COUNT] = {[0 ... RGB_MATRIX_LED_COUNT - 1] = {OPENRGB_DIRECT_MODE_STARTUP_GREEN, OPENRGB_DIRECT_MODE_STARTUP_RED, OPENRGB_DIRECT_MODE_STARTUP_BLUE}};
// NOTE: the old openrgb_rgb_matrix_effects_indexes[] table was removed.
// openrgb_get_enabled_modes() now enumerates compiled effects dynamically
// (1..RGB_MATRIX_EFFECT_MAX-1), so no static effect-index table is needed.
static uint8_t raw_hid_buffer[RAW_EPSIZE];

extern bool via_command_kb(uint8_t *data, uint8_t length);

void raw_hid_receive(uint8_t *data, uint8_t length) {
    if (*data >= 0x0a) {
        via_command_kb(data, length);
        return;
    }

    switch (*data) {
        case OPENRGB_GET_PROTOCOL_VERSION:
            openrgb_get_protocol_version();
            break;
        case OPENRGB_GET_QMK_VERSION:
            openrgb_get_qmk_version();
            break;
        case OPENRGB_GET_DEVICE_INFO:
            openrgb_get_device_info();
            break;
        case OPENRGB_GET_MODE_INFO:
            openrgb_get_mode_info();
            break;
        case OPENRGB_GET_LED_INFO:
            openrgb_get_led_info(data);
            break;
        case OPENRGB_GET_ENABLED_MODES:
            openrgb_get_enabled_modes();
            break;

        case OPENRGB_SET_MODE:
            openrgb_set_mode(data);
            break;
        case OPENRGB_DIRECT_MODE_SET_SINGLE_LED:
            openrgb_direct_mode_set_single_led(data);
            break;
        case OPENRGB_DIRECT_MODE_SET_LEDS:
            openrgb_direct_mode_set_leds(data);
            break;
    }

    if (*data != OPENRGB_DIRECT_MODE_SET_LEDS) {
        raw_hid_buffer[RAW_EPSIZE - 1] = OPENRGB_END_OF_MESSAGE;
        raw_hid_send(raw_hid_buffer, RAW_EPSIZE);
        memset(raw_hid_buffer, 0x00, RAW_EPSIZE);
    }
}

void openrgb_get_protocol_version(void) {
    raw_hid_buffer[0] = OPENRGB_GET_PROTOCOL_VERSION;
    raw_hid_buffer[1] = OPENRGB_PROTOCOL_VERSION;
}
void openrgb_get_qmk_version(void) {
    raw_hid_buffer[0]    = OPENRGB_GET_QMK_VERSION;
    uint8_t current_byte = 1;
    for (uint8_t i = 0; (current_byte < (RAW_EPSIZE - 2)) && (QMK_VERSION[i] != 0); i++) {
        raw_hid_buffer[current_byte] = QMK_VERSION[i];
        current_byte++;
    }
}
void openrgb_get_device_info(void) {
    raw_hid_buffer[0] = OPENRGB_GET_DEVICE_INFO;
    raw_hid_buffer[1] = RGB_MATRIX_LED_COUNT;
    raw_hid_buffer[2] = MATRIX_COLS * MATRIX_ROWS;

#define MASSDROP_VID 0x04D8
#if VENDOR_ID == MASSDROP_VID
#    define PRODUCT_STRING PRODUCT
#    define MANUFACTURER_STRING MANUFACTURER
#else
#    define PRODUCT_STRING STR(PRODUCT)
#    define MANUFACTURER_STRING STR(MANUFACTURER)
#endif

    uint8_t current_byte = 3;
    for (uint8_t i = 0; (current_byte < ((RAW_EPSIZE - 2) / 2)) && (PRODUCT_STRING[i] != 0); i++) {
        raw_hid_buffer[current_byte] = PRODUCT_STRING[i];
        current_byte++;
    }
    raw_hid_buffer[current_byte] = 0;
    current_byte++;

    for (uint8_t i = 0; (current_byte + 2 < RAW_EPSIZE) && (MANUFACTURER_STRING[i] != 0); i++) {
        raw_hid_buffer[current_byte] = MANUFACTURER_STRING[i];
        current_byte++;
    }
}
void openrgb_get_mode_info(void) {
    const HSV hsv_color = rgb_matrix_get_hsv();

    raw_hid_buffer[0] = OPENRGB_GET_MODE_INFO;
    raw_hid_buffer[1] = rgb_matrix_get_mode();
    raw_hid_buffer[2] = rgb_matrix_get_speed();
    raw_hid_buffer[3] = hsv_color.h;
    raw_hid_buffer[4] = hsv_color.s;
    raw_hid_buffer[5] = hsv_color.v;
}
void openrgb_get_led_info(uint8_t *data) {
    const uint8_t first_led   = data[1];
    const uint8_t number_leds = data[2];

    raw_hid_buffer[0] = OPENRGB_GET_LED_INFO;

    for (uint8_t i = 0; i < number_leds; i++) {
        const uint8_t led_idx = first_led + i;
        const uint8_t data_idx  = i * 7;

        if (led_idx >= RGB_MATRIX_LED_COUNT) {
            raw_hid_buffer[data_idx + 3] = OPENRGB_FAILURE;
        } else {
            raw_hid_buffer[data_idx + 1] = g_led_config.point[led_idx].x;
            raw_hid_buffer[data_idx + 2] = g_led_config.point[led_idx].y;
            raw_hid_buffer[data_idx + 3] = g_led_config.flags[led_idx];
            raw_hid_buffer[data_idx + 4] = g_openrgb_direct_mode_colors[led_idx].r;
            raw_hid_buffer[data_idx + 5] = g_openrgb_direct_mode_colors[led_idx].g;
            raw_hid_buffer[data_idx + 6] = g_openrgb_direct_mode_colors[led_idx].b;
        }

        uint8_t row   = 0;
        uint8_t col   = 0;
        uint8_t found = 0;

        for (row = 0; row < MATRIX_ROWS; row++) {
            for (col = 0; col < MATRIX_COLS; col++) {
                if (g_led_config.matrix_co[row][col] == led_idx) {
                    found = 1;
                    break;
                }
            }

            if (found == 1) {
                break;
            }
        }

        if (col >= MATRIX_COLS || row >= MATRIX_ROWS) {
            raw_hid_buffer[data_idx + 7] = KC_NO;
        }
        else {
            raw_hid_buffer[data_idx + 7] = (uint8_t) keycode_at_keymap_location(0, row, col);
        }
    }
}
void openrgb_get_enabled_modes(void) {
    raw_hid_buffer[0] = OPENRGB_GET_ENABLED_MODES;
    // Report the actual compiled effects. Every rgb_matrix mode from 1 up to
    // RGB_MATRIX_EFFECT_MAX-1 is a real, selectable effect in this build, so we
    // enumerate them dynamically instead of using a hard-coded table that
    // assumed a specific (canonical) effect set and drifted out of sync with
    // this fork's renamed/extra effects. This keeps the OpenRGB mode list
    // exactly in sync with what the firmware can actually play.
    uint8_t idx = 1;
    for (uint8_t mode = 1; mode < RGB_MATRIX_EFFECT_MAX && idx < RAW_EPSIZE; mode++) {
        raw_hid_buffer[idx++] = mode;
    }
}

void openrgb_set_mode(uint8_t *data) {
    const uint8_t h     = data[1];
    const uint8_t s     = data[2];
    const uint8_t v     = data[3];
    const uint8_t mode  = data[4];
    const uint8_t speed = data[5];
    const uint8_t save = data[6];

    raw_hid_buffer[0] = OPENRGB_SET_MODE;

    if (h > 255 || s > 255 || v > 255 || mode >= RGB_MATRIX_EFFECT_MAX || speed > 255) {
        raw_hid_buffer[RAW_EPSIZE - 2] = OPENRGB_FAILURE;
        return;
    }

    if (save == 1) {
        rgb_matrix_mode(mode	);
        rgb_matrix_set_speed(speed);
        rgb_matrix_sethsv(h, s, v);
    }
    else {
        rgb_matrix_mode_noeeprom( mode	);
        rgb_matrix_set_speed_noeeprom(speed);
        rgb_matrix_sethsv_noeeprom(h, s, v);
    }

    raw_hid_buffer[RAW_EPSIZE - 2] = OPENRGB_SUCCESS;
}
void openrgb_direct_mode_set_single_led(uint8_t *data) {
    const uint8_t led = data[1];
    const uint8_t r   = data[2];
    const uint8_t g   = data[3];
    const uint8_t b   = data[4];

    raw_hid_buffer[0] = OPENRGB_DIRECT_MODE_SET_SINGLE_LED;

    if (led >= RGB_MATRIX_LED_COUNT || r > 255 || g > 255 || b > 255) {
        raw_hid_buffer[RAW_EPSIZE - 2] = OPENRGB_FAILURE;
        return;
    }

    // OpenRGB streams direct colors without first selecting our effect, so make
    // sure the matrix is in OPENRGB_DIRECT whenever direct data arrives.
    if (rgb_matrix_get_mode() != RGB_MATRIX_OPENRGB_DIRECT) {
        rgb_matrix_mode_noeeprom(RGB_MATRIX_OPENRGB_DIRECT);
    }

    g_openrgb_direct_mode_colors[led].r = r;
    g_openrgb_direct_mode_colors[led].g = g;
    g_openrgb_direct_mode_colors[led].b = b;

    raw_hid_buffer[RAW_EPSIZE - 2] = OPENRGB_SUCCESS;
}
void openrgb_direct_mode_set_leds(uint8_t *data) {
    // OpenRGB streams direct colors without first selecting our effect, so make
    // sure the matrix is in OPENRGB_DIRECT whenever direct data arrives.
    if (rgb_matrix_get_mode() != RGB_MATRIX_OPENRGB_DIRECT) {
        rgb_matrix_mode_noeeprom(RGB_MATRIX_OPENRGB_DIRECT);
    }

    const uint8_t number_leds = data[1];

    for (uint8_t i = 0; i < number_leds; i++) {
        const uint8_t data_idx  = i * 4;
        const uint8_t color_idx = data[data_idx + 2];

        g_openrgb_direct_mode_colors[color_idx].r = data[data_idx + 3];
        g_openrgb_direct_mode_colors[color_idx].g = data[data_idx + 4];
        g_openrgb_direct_mode_colors[color_idx].b = data[data_idx + 5];
    }
}
