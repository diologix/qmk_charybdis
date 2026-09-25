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
#define ______________HOME_ROW_GACS_L______________ KC_LGUI, KC_LALT, KC_LCTL, KC_LSFT, XXXXXXX
#define ______________HOME_ROW_GACS_R______________ XXXXXXX, KC_LSFT, KC_LCTL, KC_LALT, KC_LGUI

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
    _______________DEAD_HALF_ROW_______________, KC_PSCR,   KC_F1,   KC_F2,   KC_F3,  KC_F10, \
    ______________HOME_ROW_GACS_L______________, KC_SCRL,   KC_F4,   KC_F5,   KC_F6,  KC_F11, \
    _______________DEAD_HALF_ROW_______________, KC_PAUS,   KC_F7,   KC_F8,   KC_F9,  KC_F12, \
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
    ______________HOME_ROW_GACS_L______________, ______________HOME_ROW_GACS_R______________, \
    _______, DRGSCRL, SNIPING, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, SNIPING, DRGSCRL, _______, \
                      MS_BTN2, MS_BTN1, MS_BTN3, MS_BTN3, MS_BTN1

/**
 * \brief Navigation layer.
 *
 * Right-hand layer (outer right thumb) is navigation and editing. Cursor
 * keys are on the home position, line and page movement below, clipboard above,
 * caps lock and insert on the inner column. Thumb keys are duplicated from the
 * base layer to avoid having to layer change mid edit and to enable auto-repeat.
 */
#define LAYOUT_LAYER_NAVIGATION                                                               \
    _______________DEAD_HALF_ROW_______________, _______________DEAD_HALF_ROW_______________, \
    ______________HOME_ROW_GACS_L______________, KC_CAPS, KC_LEFT, KC_DOWN,   KC_UP, KC_RGHT, \
    _______________DEAD_HALF_ROW_______________,  KC_INS, KC_HOME, KC_PGDN, KC_PGUP,  KC_END, \
                       KC_TAB,  KC_SPC,  KC_ENT, KC_BSPC, _______

/**
 * \brief Numeral layout.
 *
 * Left-hand layer (outer left thumb) is numerals and symbols. Numerals are in
 * the standard numpad locations with symbols in the remaining positions.
 * `KC_DOT` is duplicated from the base layer.
 */
#define LAYOUT_LAYER_NUMERAL                                                                  \
    KC_LBRC,    KC_1,    KC_2,    KC_3, KC_RBRC, _______________DEAD_HALF_ROW_______________, \
    KC_SCLN,    KC_4,    KC_5,    KC_6,  KC_EQL, ______________HOME_ROW_GACS_R______________, \
     KC_GRV,    KC_7,    KC_8,    KC_9, KC_BSLS, _______________DEAD_HALF_ROW_______________, \
                      _______,    KC_0, KC_MINS,  KC_DOT, XXXXXXX

/**
 * \brief Symbols layer.
 *
 * Secondary left-hand layer has shifted symbols in the same locations to reduce
 * chording when using mods with shifted symbols. `KC_LPRN` is duplicated next to
 * `KC_RPRN`.
 */
#define LAYOUT_LAYER_SYMBOLS                                                                  \
    KC_LCBR, KC_AMPR, KC_ASTR, KC_LPRN, KC_RCBR, _______________DEAD_HALF_ROW_______________, \
    KC_COLN,  KC_DLR, KC_PERC, KC_CIRC, KC_PLUS, ______________HOME_ROW_GACS_R______________, \
    KC_TILD, KC_EXLM,   KC_AT, KC_HASH, KC_PIPE, _______________DEAD_HALF_ROW_______________, \
                      KC_LPRN, KC_RPRN, KC_UNDS, _______, XXXXXXX

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
 * Expects a 10-key per row layout.  Adds support for GACS (Gui, Alt, Ctl, Shift)
 * home row.  The layout passed in parameter must contain at least 20 keycodes.
 *
 * This is meant to be used with `LAYER_ALPHAS_QWERTY` defined above, eg.:
 *
 *     HOME_ROW_MOD_GACS(LAYER_ALPHAS_QWERTY)
 */
#define _HOME_ROW_MOD_GACS(                                            \
    L00, L01, L02, L03, L04, R05, R06, R07, R08, R09,                  \
    L10, L11, L12, L13, L14, R15, R16, R17, R18, R19,                  \
    ...)                                                               \
             L00,         L01,         L02,         L03,         L04,  \
             R05,         R06,         R07,         R08,         R09,  \
      LGUI_T(L10), LALT_T(L11), LCTL_T(L12), LSFT_T(L13),        L14,  \
             R15,  RSFT_T(R16), RCTL_T(R17), LALT_T(R18), RGUI_T(R19), \
      __VA_ARGS__
#define HOME_ROW_MOD_GACS(...) _HOME_ROW_MOD_GACS(__VA_ARGS__)

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
    POINTER_MOD(HOME_ROW_MOD_GACS(LAYOUT_LAYER_BASE))
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
const uint16_t PROGMEM rst_combo[] = {LALT_T(KC_R), LCTL_T(KC_S), LSFT_T(KC_T), COMBO_END};

combo_t key_combos[] = {
    COMBO(rst_combo, KC_ESC),
};

#ifdef POINTING_DEVICE_ENABLE
/**
 * \brief Reverse the vertical drag-scroll direction.
 *
 * Uses the pointing-device module's own invert setting, which it persists to
 * EEPROM.  Runs from the main loop, i.e. after the module has loaded its
 * config, and only writes when the setting is off (e.g. after `EE_CLR`).
 */
void housekeeping_task_user(void) {
    if (!bkpd_mode_get_invert(MODE_DRAGSCROLL, 1)) {
        bkpd_mode_set_invert(MODE_DRAGSCROLL, 1, true);
    }
}
#endif // POINTING_DEVICE_ENABLE
