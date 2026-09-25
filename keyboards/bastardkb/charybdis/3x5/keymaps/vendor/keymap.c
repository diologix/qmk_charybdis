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
#    include "argos_combo.h"
#    include "dynamic_keymap.h"
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

// Tap: left click (on every layer), hold: numeral layer.
#define BTN_NUM LT(LAYER_NUMERAL, MS_BTN1)
#define SPC_NAV LT(LAYER_NAVIGATION, KC_SPC)
#define ENT_FUN LT(LAYER_FUNCTION, KC_ENT)
#define BSP_SYM LT(LAYER_SYMBOLS, KC_BSPC)
// Tap: Tab, hold: navigation layer.
#define TAB_NAV LT(LAYER_NAVIGATION, KC_TAB)
#define _L_PTR(KC) LT(LAYER_POINTER, KC)
// Top outer keys: hold for the media layer.
#define MED_Q LT(LAYER_MEDIA, KC_Q)
#define MED_SCL LT(LAYER_MEDIA, KC_SCLN)

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

// Pointer layer: tap = middle click, hold = drag-scroll.  Handled in
// `process_record_user`.
#define BTN3_SCR LT(0, MS_BTN3)

#ifndef POINTING_DEVICE_ENABLE
#    define DRGSCRL KC_NO
#    define DPI_MOD KC_NO
#    define S_D_MOD KC_NO
#    define SNIPING KC_NO
#endif // !POINTING_DEVICE_ENABLE

// clang-format off
/** \brief QWERTY layout (3 rows, 10 columns). */
#define LAYOUT_LAYER_BASE                                                                     \
      MED_Q,    KC_W,    KC_F,    KC_P,    KC_B,    KC_J,    KC_L,    KC_U,    KC_Y,    MED_SCL, \
       KC_A,    KC_R,    KC_S,    KC_T,    KC_G,    KC_M,    KC_N,    KC_E,    KC_I, KC_O, \
       KC_Z,    KC_X,    KC_C,    KC_D,    KC_V,    KC_K,    KC_H, KC_COMM,  KC_DOT, KC_SLSH, \
                      BTN_NUM, SPC_NAV, ENT_FUN, BSP_SYM, TAB_NAV

/** Convenience row shorthands. */
#define _______________DEAD_HALF_ROW_______________ XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX
#define ______________HOME_ROW_SCGA_L______________ KC_LSFT, KC_LCTL, KC_LGUI, KC_LALT, XXXXXXX

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
 * \brief Media layer (hold the top outer key on either side).
 *
 * Left half: the lower two rows of the ZMK media layer's right half.  Right
 * half keeps previous/volume/mute/next.  Bootloader and EEPROM reset on the
 * inner bottom keys.
 */
#define LAYOUT_LAYER_MEDIA                                                                    \
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, _______________DEAD_HALF_ROW_______________, \
    KC_MPRV, KC_VOLD, KC_VOLU, KC_MNXT, XXXXXXX, KC_MPRV, KC_VOLD, KC_MUTE, KC_VOLU, KC_MNXT, \
    XXXXXXX, KC_MUTE, KC_MPLY,  EE_CLR, QK_BOOT, QK_BOOT,  EE_CLR, XXXXXXX, XXXXXXX, XXXXXXX, \
                      KC_MPLY, _______, KC_MSTP, KC_MSTP, KC_MPLY

/**
 * \brief Mouse emulation and pointer functions.
 *
 * Right home row: browser back/forward (mouse buttons 4/5) on M/I, page
 * down/up on N/E, Shift on O.
 */
#define LAYOUT_LAYER_POINTER                                                                  \
    QK_BOOT,  EE_CLR, XXXXXXX, DPI_MOD, S_D_MOD, S_D_MOD, DPI_MOD, XXXXXXX,  EE_CLR, QK_BOOT, \
    ______________HOME_ROW_SCGA_L______________, MS_BTN4, KC_PGDN, KC_PGUP, MS_BTN5, KC_LSFT, \
    _______, DRGSCRL, SNIPING, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, SNIPING, DRGSCRL, _______, \
                      MS_BTN1, BTN3_SCR, MS_BTN2, MS_BTN1, MS_BTN2

/**
 * \brief Navigation layer.
 *
 * Left half taken over from the ZMK keymap (L1-NAV): workspace keys (tap =
 * Gui+n, hold = Gui+Shift+n), Gui+Alt+1/2 and Shift.  Right half has the
 * arrows starting on the inner column as in ZMK, line and page movement below,
 * caps lock above.  Held from the middle left or the outer right thumb.
 */
#define LAYOUT_LAYER_NAVIGATION                                                               \
    XXXXXXX,    WS_1,    WS_2,    WS_3, LGUI(LALT(KC_1)), KC_CAPS, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, \
       WS_0,    WS_4,    WS_5,    WS_6, LGUI(LALT(KC_2)), KC_LEFT, KC_DOWN,   KC_UP, KC_RGHT, XXXXXXX, \
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
};

// clang-format on

/** \brief Combos. R, S and T are home-row mod-taps, so match those keycodes. */
const uint16_t PROGMEM rst_combo[] = {LCTL_T(KC_R), LGUI_T(KC_S), LALT_T(KC_T), COMBO_END};
const uint16_t PROGMEM st_combo[]  = {LGUI_T(KC_S), LALT_T(KC_T), COMBO_END};
const uint16_t PROGMEM tab_combo[]       = {LSFT_T(KC_A), LCTL_T(KC_R), COMBO_END};
const uint16_t PROGMEM shift_tab_combo[] = {MED_Q, KC_W, COMBO_END};
// Clipboard combos from the ZMK keymap: letter + Space thumb = Ctrl+letter,
// letter + outer left thumb = Ctrl+Shift+letter.
const uint16_t PROGMEM copy_combo[]        = {KC_C, SPC_NAV, COMBO_END};
const uint16_t PROGMEM paste_combo[]       = {KC_V, SPC_NAV, COMBO_END};
const uint16_t PROGMEM shift_copy_combo[]  = {KC_C, BTN_NUM, COMBO_END};
const uint16_t PROGMEM shift_paste_combo[] = {KC_V, BTN_NUM, COMBO_END};
const uint16_t PROGMEM username_combo[]    = {KC_B, SPC_NAV, COMBO_END};
// Umlauts from the ZMK keymap, sent as AltGr combos for US International.
// Hold Shift on the other hand for capitals.
const uint16_t PROGMEM ae_combo[] = {LSFT_T(KC_A), SPC_NAV, COMBO_END};
const uint16_t PROGMEM sz_combo[] = {LGUI_T(KC_S), SPC_NAV, COMBO_END};
const uint16_t PROGMEM oe_combo[] = {LSFT_T(KC_O), TAB_NAV, COMBO_END};
const uint16_t PROGMEM ue_combo[] = {KC_U, TAB_NAV, COMBO_END};

combo_t key_combos[] = {
    COMBO(rst_combo, KC_ESC),
    // Sticky Gui for the next key, as ZMK's `&sk LEFT_WIN`.
    COMBO(st_combo, OSM(MOD_LGUI)),
    COMBO(tab_combo, KC_TAB),
    COMBO(shift_tab_combo, LSFT(KC_TAB)),
    COMBO(copy_combo, LCTL(KC_C)),
    COMBO(paste_combo, LCTL(KC_V)),
    COMBO(shift_copy_combo, LCTL(LSFT(KC_C))),
    COMBO(shift_paste_combo, LCTL(LSFT(KC_V))),
    // Copy username (Ctrl+B, e.g. in KeePassXC).
    COMBO(username_combo, LCTL(KC_B)),
    COMBO(ae_combo, ALGR(KC_Q)),
    COMBO(sz_combo, ALGR(KC_S)),
    COMBO(oe_combo, ALGR(KC_P)),
    COMBO(ue_combo, ALGR(KC_Y)),
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case WS_1 ... WS_0:
            if (record->event.pressed) {
                uint16_t kc = QK_LAYER_TAP_GET_TAP_KEYCODE(keycode);
                tap_code16(record->tap.count ? LGUI(kc) : LGUI(LSFT(kc)));
            }
            return false;
#ifdef POINTING_DEVICE_ENABLE
        case BTN3_SCR:
            if (record->tap.count) {
                if (record->event.pressed) {
                    tap_code16(MS_BTN3);
                }
            } else if (record->event.pressed) {
                bkpd_mode_set_active(MODE_DRAGSCROLL);
            } else {
                bkpd_mode_release(MODE_DRAGSCROLL);
            }
            return false;
#endif // POINTING_DEVICE_ENABLE
    }
    return true;
}

/**
 * \brief Make the firmware authoritative over settings kept in EEPROM.
 *
 * Argos (the pointing-device module depends on it) and VIA keep the keymap,
 * combos and tapping term in EEPROM and only seed them from the firmware once.
 * Re-seed them on every boot so changes to this keymap always take effect.
 * Runs once, from the main loop, i.e. after the modules have loaded their
 * config.  DPI keys on the pointer layer still work until the next restart.
 */
void housekeeping_task_user(void) {
    static bool initialized = false;
    if (initialized) {
        return;
    }
    initialized = true;
#ifdef COMMUNITY_MODULE_ARGOS_ENABLE
    dynamic_keymap_reset();
    argos_combos_copy_from_QMK();
    argos_combos_load_from_eeprom();
    // Argos' `get_tapping_term` returns its own value (RAM only, no write).
    argos_config.global_tapping_term = TAPPING_TERM;
#endif // COMMUNITY_MODULE_ARGOS_ENABLE
#ifdef POINTING_DEVICE_ENABLE
    // Reverse the vertical drag-scroll direction; 500 DPI, 200 when sniping.
    if (!bkpd_mode_get_invert(MODE_DRAGSCROLL, 1)) {
        bkpd_mode_set_invert(MODE_DRAGSCROLL, 1, true);
    }
    if (bkpd_mode_get_dpi(MODE_NORMAL) != 500) {
        bkpd_mode_change_dpi(MODE_NORMAL, 500);
    }
    if (bkpd_mode_get_dpi(MODE_SNIPING) != 200) {
        bkpd_mode_change_dpi(MODE_SNIPING, 200);
    }
#endif // POINTING_DEVICE_ENABLE
}
