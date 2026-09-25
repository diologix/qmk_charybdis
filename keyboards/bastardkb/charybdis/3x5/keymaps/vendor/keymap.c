/**
 * Copyright 2021 Charly Delay <charly@codesink.dev> (@0xcharly)
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
#include QMK_KEYBOARD_H

#ifdef POINTING_DEVICE_ENABLE
#    include "bk_pointing_device.h"
#endif // POINTING_DEVICE_ENABLE

#ifdef COMMUNITY_MODULE_ARGOS_ENABLE
#    include "argos.h"
extern argos_config_t argos_config;
#endif // COMMUNITY_MODULE_ARGOS_ENABLE

#ifdef CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_ENABLE
#    include "timer.h"
#endif // CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_ENABLE

enum charybdis_keymap_layers {
    LAYER_BASE = 0,
    LAYER_FUNCTION,
    LAYER_NAVIGATION,
    LAYER_MEDIA,
    LAYER_POINTER,
    LAYER_NUMERAL,
    LAYER_SYMBOLS,
    LAYER_UMLAUT,
};

// Automatically enable sniping-mode on the pointer layer.
#define CHARYBDIS_AUTO_SNIPING_ON_LAYER LAYER_POINTER

#ifdef CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_ENABLE
static uint16_t auto_pointer_layer_timer = 0;

#    ifndef CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_TIMEOUT_MS
#        define CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_TIMEOUT_MS 500
#    endif // CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_TIMEOUT_MS

#    ifndef CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_THRESHOLD
#        define CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_THRESHOLD 8
#    endif // CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_THRESHOLD
#endif     // CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_ENABLE

#define TAB_NUM LT(LAYER_NUMERAL, KC_TAB)
#define SPC_MED LT(LAYER_MEDIA, KC_SPC)
#define ENT_FUN LT(LAYER_FUNCTION, KC_ENT)
#define BSP_SYM LT(LAYER_SYMBOLS, KC_BSPC)
// Tap: left click (on every layer), hold: navigation layer.
#define BTN_NAV LT(LAYER_NAVIGATION, MS_BTN1)
#define _L_PTR(KC) LT(LAYER_POINTER, KC)
#define UML_SCL LT(LAYER_UMLAUT, KC_SCLN)

// Workspace keys (nav layer): tap = Gui+n, hold = Gui+Shift+n.  Handled in
// `process_record_user`; layer 0 is only used as a tap-hold carrier.
#define WS_1 LT(0, KC_1)
#define WS_2 LT(0, KC_2)
#define WS_3 LT(0, KC_3)
#define WS_4 LT(0, KC_4)
#define WS_5 LT(0, KC_5)
#define WS_6 LT(0, KC_6)
#define WS_7 LT(0, KC_7)
#define WS_8 LT(0, KC_8)
#define WS_9 LT(0, KC_9)
#define WS_0 LT(0, KC_0)

#ifndef POINTING_DEVICE_ENABLE
#    define DRGSCRL KC_NO
#    define DPI_MOD KC_NO
#    define S_D_MOD KC_NO
#    define SNIPING KC_NO
#endif // !POINTING_DEVICE_ENABLE

// clang-format off
/** \brief QWERTY layout (3 rows, 10 columns). */
#define LAYOUT_LAYER_BASE                                                                     \
       KC_Q,    KC_W,    KC_F,    KC_P,    KC_B,    KC_J,    KC_L,    KC_U,    KC_Y,    UML_SCL, \
       KC_A,    KC_R,    KC_S,    KC_T,    KC_G,    KC_M,    KC_N,    KC_E,    KC_I, KC_O, \
       KC_Z,    KC_X,    KC_C,    KC_D,    KC_V,    KC_K,    KC_H, KC_COMM,  KC_DOT, KC_SLSH, \
                      TAB_NUM, SPC_MED, ENT_FUN, BSP_SYM, BTN_NAV

/** Convenience row shorthands. */
#define _______________DEAD_HALF_ROW_______________ XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX
#define ______________HOME_ROW_SCGA_L______________ KC_LSFT, KC_LCTL, KC_LGUI, KC_LALT, XXXXXXX
#define ______________HOME_ROW_SCGA_R______________ XXXXXXX, KC_LCTL, KC_LGUI, KC_LALT, KC_LSFT

/*
 * Layers used on the Charybdis Nano.
 *
 * These layers started off heavily inspired by the Miryoku layout, but trimmed
 * down and tailored for a stock experience that is meant to be fundation for
 * further personalization.
 *
 * See https://github.com/manna-harbour/miryoku for the original layout.
 */

/**
 * \brief Function layer.
 *
 * Secondary right-hand layer has function keys mirroring the numerals on the
 * primary layer with extras on the pinkie column, plus system keys on the inner
 * column. App is on the tertiary thumb key and other thumb keys are duplicated
 * from the base layer to enable auto-repeat.
 */
#define LAYOUT_LAYER_FUNCTION                                                                 \
    _______________DEAD_HALF_ROW_______________, KC_F10,   KC_F1,   KC_F2,   KC_F3, KC_PAUS, \
    ______________HOME_ROW_SCGA_L______________, KC_F11,   KC_F4,   KC_F5,   KC_F6, KC_SCRL, \
    _______________DEAD_HALF_ROW_______________, KC_F12,   KC_F7,   KC_F8,   KC_F9, KC_PSCR, \
                      XXXXXXX, XXXXXXX, _______, XXXXXXX, XXXXXXX

/**
 * \brief Media layer.
 *
 * Tertiary left- and right-hand layer is media and RGB control.  This layer is
 * symmetrical to accomodate the left- and right-hand trackball.
 */
#define LAYOUT_LAYER_MEDIA                                                                    \
    XXXXXXX,RM_PREV, RM_TOGG, RM_NEXT, XXXXXXX, XXXXXXX,RM_PREV, RM_TOGG, RM_NEXT, XXXXXXX, \
    KC_MPRV, KC_VOLD, KC_MUTE, KC_VOLU, KC_MNXT, KC_MPRV, KC_VOLD, KC_MUTE, KC_VOLU, KC_MNXT, \
    XXXXXXX, XXXXXXX, XXXXXXX,  EE_CLR, QK_BOOT, QK_BOOT,  EE_CLR, XXXXXXX, XXXXXXX, XXXXXXX, \
                      KC_MPLY, _______, KC_MSTP, KC_MSTP, KC_MPLY

/** \brief Mouse emulation and pointer functions. */
#define LAYOUT_LAYER_POINTER                                                                  \
    QK_BOOT,  EE_CLR, XXXXXXX, DPI_MOD, S_D_MOD, S_D_MOD, DPI_MOD, XXXXXXX,  EE_CLR, QK_BOOT, \
    ______________HOME_ROW_SCGA_L______________, ______________HOME_ROW_SCGA_R______________, \
    _______, DRGSCRL, SNIPING, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, SNIPING, DRGSCRL, _______, \
                      MS_BTN2, MS_BTN1, MS_BTN3, MS_BTN3, MS_BTN1

/**
 * \brief Navigation layer.
 *
 * Left half taken over from the ZMK keymap (L1-NAV): workspace keys (tap =
 * Gui+n, hold = Gui+Shift+n), Gui+Alt+1/2 and Shift.  Right half has the
 * arrows starting on the inner column as in ZMK, line and page movement below.
 */
#define LAYOUT_LAYER_NAVIGATION                                                               \
    XXXXXXX,    WS_1,    WS_2,    WS_3, LGUI(LALT(KC_1)), _______________DEAD_HALF_ROW_______________, \
       WS_0,    WS_4,    WS_5,    WS_6, LGUI(LALT(KC_2)), KC_LEFT, KC_DOWN,   KC_UP, KC_RGHT, KC_CAPS, \
    KC_LSFT,    WS_7,    WS_8,    WS_9, XXXXXXX, KC_HOME, KC_PGDN, KC_PGUP,  KC_END,  KC_INS, \
                      KC_LSFT, _______, KC_LGUI, KC_BSPC, _______

/**
 * \brief Numeral layout.
 *
 * Taken over from the ZMK keymap (L5_NUMBERS), outer columns dropped.  Thumb
 * keys are transparent.
 */
#define LAYOUT_LAYER_NUMERAL                                                                  \
    KC_UNDS,    KC_1,    KC_2,    KC_3, KC_PLUS,  KC_NUM, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, \
       KC_0,    KC_4,    KC_5,    KC_6, KC_MINS, _______________DEAD_HALF_ROW_______________, \
     KC_DOT,    KC_7,    KC_8,    KC_9,  KC_EQL, _______________DEAD_HALF_ROW_______________, \
                      _______, _______, _______, _______, _______

/**
 * \brief Symbols layer.
 *
 * Taken over from the ZMK keymap (L3_SYMBOLS), outer columns dropped.  Thumb
 * keys are transparent.
 */
#define LAYOUT_LAYER_SYMBOLS                                                                  \
    KC_HASH, KC_EXLM, KC_DQUO, KC_QUOT, KC_MINS, XXXXXXX, XXXXXXX, XXXXXXX, KC_AMPR, KC_ASTR, \
    KC_TILD,  KC_DLR, KC_PERC, KC_CIRC, KC_PIPE, _______________DEAD_HALF_ROW_______________, \
      KC_AT, KC_LCBR, KC_RCBR, KC_LBRC, KC_RBRC, KC_LPRN, KC_RPRN, XXXXXXX, XXXXXXX, KC_BSLS, \
                      _______, _______, _______, _______, _______

/**
 * \brief German umlaut layer (hold the top-right key).
 *
 * Umlauts on the A, O, U and S positions, sent as AltGr combos for the
 * US International layout: AltGr+Q=ä, AltGr+P=ö, AltGr+Y=ü, AltGr+S=ß.
 * All other keys are transparent, so the base layer's home-row Shift can be
 * held for capitals.
 */
#define LAYOUT_LAYER_UMLAUT                                                                   \
    _______, _______, _______, _______, _______, _______, _______, ALGR(KC_Y), _______, _______, \
    ALGR(KC_Q), _______, ALGR(KC_S), _______, _______, _______, _______, _______, _______, ALGR(KC_P), \
    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, \
                      _______, _______, _______, _______, _______

/**
 * \brief Add Home Row mod to a layout.
 *
 * Expects a 10-key per row layout.  Adds SCGA (Shift, Ctl, Gui, Alt) home-row
 * mods, mirrored on the right, as in the ZMK keymap.  The layout passed in
 * parameter must contain at least 20 keycodes.
 *
 * This is meant to be used with `LAYER_ALPHAS_QWERTY` defined above, eg.:
 *
 *     HOME_ROW_MOD_SCGA(LAYER_ALPHAS_QWERTY)
 */
#define _HOME_ROW_MOD_SCGA(                                            \
    L00, L01, L02, L03, L04, R05, R06, R07, R08, R09,                  \
    L10, L11, L12, L13, L14, R15, R16, R17, R18, R19,                  \
    ...)                                                               \
             L00,         L01,         L02,         L03,         L04,  \
             R05,         R06,         R07,         R08,         R09,  \
      LSFT_T(L10), LCTL_T(L11), LGUI_T(L12), LALT_T(L13),        L14,  \
             R15,  LCTL_T(R16), LGUI_T(R17), LALT_T(R18), LSFT_T(R19), \
      __VA_ARGS__
#define HOME_ROW_MOD_SCGA(...) _HOME_ROW_MOD_SCGA(__VA_ARGS__)

/**
 * \brief Add pointer layer keys to a layout.
 *
 * Expects a 10-key per row layout.  The layout passed in parameter must contain
 * at least 30 keycodes.
 *
 * This is meant to be used with `LAYER_ALPHAS_QWERTY` defined above, eg.:
 *
 *     POINTER_MOD(LAYER_ALPHAS_QWERTY)
 */
#define _POINTER_MOD(                                                  \
    L00, L01, L02, L03, L04, R05, R06, R07, R08, R09,                  \
    L10, L11, L12, L13, L14, R15, R16, R17, R18, R19,                  \
    L20, L21, L22, L23, L24, R25, R26, R27, R28, R29,                  \
    ...)                                                               \
             L00,         L01,         L02,         L03,         L04,  \
             R05,         R06,         R07,         R08,         R09,  \
             L10,         L11,         L12,         L13,         L14,  \
             R15,         R16,         R17,         R18,         R19,  \
      _L_PTR(L20),        L21,         L22,         L23,         L24,  \
             R25,         R26,         R27,         R28,  _L_PTR(R29), \
      __VA_ARGS__
#define POINTER_MOD(...) _POINTER_MOD(__VA_ARGS__)

#define LAYOUT_wrapper(...) LAYOUT(__VA_ARGS__)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [LAYER_BASE] = LAYOUT_wrapper(
    POINTER_MOD(HOME_ROW_MOD_SCGA(LAYOUT_LAYER_BASE))
  ),
  [LAYER_FUNCTION] = LAYOUT_wrapper(LAYOUT_LAYER_FUNCTION),
  [LAYER_NAVIGATION] = LAYOUT_wrapper(LAYOUT_LAYER_NAVIGATION),
  [LAYER_MEDIA] = LAYOUT_wrapper(LAYOUT_LAYER_MEDIA),
  [LAYER_NUMERAL] = LAYOUT_wrapper(LAYOUT_LAYER_NUMERAL),
  [LAYER_POINTER] = LAYOUT_wrapper(LAYOUT_LAYER_POINTER),
  [LAYER_SYMBOLS] = LAYOUT_wrapper(LAYOUT_LAYER_SYMBOLS),
  [LAYER_UMLAUT] = LAYOUT_wrapper(LAYOUT_LAYER_UMLAUT),
};

// clang-format on

/** \brief Combos. R, S and T are home-row mod-taps, so match those keycodes. */
const uint16_t PROGMEM rst_combo[] = {LCTL_T(KC_R), LGUI_T(KC_S), LALT_T(KC_T), COMBO_END};

combo_t key_combos[] = {
    COMBO(rst_combo, KC_ESC),
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case WS_1 ... WS_0:
            if (record->event.pressed) {
                uint16_t kc = QK_LAYER_TAP_GET_TAP_KEYCODE(keycode);
                tap_code16(record->tap.count ? LGUI(kc) : LGUI(LSFT(kc)));
            }
            return false;
    }
    return true;
}

/**
 * \brief Enforce settings that modules otherwise read from EEPROM.
 *
 * Runs from the main loop, i.e. after the modules have loaded their config.
 */
void housekeeping_task_user(void) {
#ifdef COMMUNITY_MODULE_ARGOS_ENABLE
    // Argos' `get_tapping_term` returns its own value; keep it at TAPPING_TERM
    // (RAM only, no EEPROM write).
    argos_config.global_tapping_term = TAPPING_TERM;
#endif // COMMUNITY_MODULE_ARGOS_ENABLE
#ifdef POINTING_DEVICE_ENABLE
    // Reverse the vertical drag-scroll direction.  Uses the pointing-device
    // module's own invert setting, persisted to EEPROM; only writes when the
    // setting is off (e.g. after `EE_CLR`).
    if (!bkpd_mode_get_invert(MODE_DRAGSCROLL, 1)) {
        bkpd_mode_set_invert(MODE_DRAGSCROLL, 1, true);
    }
    // Start with 500 DPI in normal mode.  Only once per boot, so the DPI key
    // on the pointer layer still works until the next restart.
    static bool dpi_initialized = false;
    if (!dpi_initialized) {
        dpi_initialized = true;
        if (bkpd_mode_get_dpi(MODE_NORMAL) != 500) {
            bkpd_mode_change_dpi(MODE_NORMAL, 500);
        }
    }
#endif // POINTING_DEVICE_ENABLE
}
