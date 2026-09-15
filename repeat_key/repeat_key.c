// Copyright 2022-2026 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file repeat_key.c
 * @brief Repeat Key community module implementation
 *
 * For full documentation, see
 * <https://getreuer.info/posts/keyboards/repeat-key>
 */

#include "repeat_key.h"

#pragma message \
    "getreuer/repeat_key: Repeat Key has evolved into core QMK feature Repeat Key! To use it, update your QMK set up and see https://docs.qmk.fm/features/repeat_key"

ASSERT_COMMUNITY_MODULES_MIN_API_VERSION(1, 0, 0);

#ifdef REPEAT_KEY_ENABLE
#error "getreuer/repeat_key: This module is an alternative to QMK core's built-in Repeat Key feature (`REPEAT_KEY_ENABLE = yes`). Please do not enable both at the same time."
#elif !defined(COMBO_ENABLE)
// This library makes use of keyrecord_t's `.keycode` field. This field is only
// present when Combos are enabled, which we check here. Enable Combos in your
// rules.mk by setting:
//   COMBO_ENABLE = yes
#error "getreuer/repeat_key: Please set `COMBO_ENABLE = yes` in rules.mk."
#else

// Variables saving the state of the last key press.
static keyrecord_t last_record = {0};
static uint8_t last_mods = 0;
// Signed count of the number of times the last key has been repeated or
// alternate repeated: it is 0 when a key is pressed normally, positive when
// repeated, and negative when alternate repeated.
static int8_t last_repeat_count = 0;

// The repeat_count, but set to 0 outside of repeat_key_invoke() so that it is
// nonzero only while a repeated key is being processed.
static int8_t processing_repeat_count = 0;

/** @brief Updates `last_repeat_count` in direction `dir`. */
static void update_last_repeat_count(int8_t dir) {
  if (dir * last_repeat_count < 0) {
    last_repeat_count = dir;
  } else if (dir * last_repeat_count < 127) {
    last_repeat_count += dir;
  }
}

void reset_repeat_key_state(void) {
  last_record = (keyrecord_t){0};
  last_mods = 0;
  last_repeat_count = 0;
}

const keyrecord_t* get_last_record(void) { return &last_record; }

void set_last_record(uint16_t keycode, keyrecord_t* record) {
  if (record) {
    last_record = *record;
  } else {
    last_record = (keyrecord_t){0};
  }
  last_record.keycode = keycode;
  last_repeat_count = 0;
}

void set_last_keycode(uint16_t keycode) {
  keyrecord_t record = {
#ifndef NO_ACTION_TAPPING
      .tap.interrupted = false,
      .tap.count = 1,
#endif
  };
  set_last_record(keycode, &record);
}

void set_last_mods(uint8_t mods) { last_mods = mods; }

uint16_t get_last_keycode(void) { return last_record.keycode; }

uint8_t get_last_mods(void) { return last_mods; }

int8_t get_repeat_key_count(void) { return processing_repeat_count; }

void repeat_key_invoke(const keyevent_t* event) {
  // It is possible (e.g. in rolled presses) that the last key changes while the
  // Repeat Key is pressed. To prevent stuck keys, it is important to remember
  // separately what key record was processed on press so that the corresponding
  // record is generated on release.
  static keyrecord_t registered_record = {0};
  static int8_t registered_repeat_count = 0;
  // Since this function calls process_record(), it may recursively call itself.
  // We return early if `processing_repeat_count` is nonzero to prevent infinite
  // recursion.
  if (processing_repeat_count || !last_record.keycode) {
    return;
  }

  if (event->pressed) {
    update_last_repeat_count(1);
    // On press, apply the last mods state, stacking on top of current mods.
    register_weak_mods(last_mods);
    registered_record = last_record;
    registered_repeat_count = last_repeat_count;
  }

  // Generate a keyrecord and plumb it into the event pipeline.
  registered_record.event = *event;
  processing_repeat_count = registered_repeat_count;
  process_record(&registered_record);
  processing_repeat_count = 0;

  // On release, restore the mods state.
  if (!event->pressed) {
    unregister_weak_mods(last_mods);
  }
}

void repeat_key_register(void) {
  repeat_key_invoke(&MAKE_KEYEVENT(0, 0, true));
}

void repeat_key_unregister(void) {
  repeat_key_invoke(&MAKE_KEYEVENT(0, 0, false));
}

void repeat_key_tap(void) {
  repeat_key_register();
  wait_ms(TAP_CODE_DELAY);
  repeat_key_unregister();
}

#ifndef NO_ALT_REPEAT_KEY

/**
 * @brief Find alternate keycode from a table of opposing keycode pairs.
 * @param table Array of pairs of basic keycodes, declared as PROGMEM.
 * @param table_size_bytes The size of the table in bytes.
 * @param target The basic keycode to find.
 * @return The alternate basic keycode, or KC_NO if none was found.
 */
static uint8_t find_alt_keycode(const uint8_t (*table)[2],
                                uint8_t table_size_bytes, uint8_t target) {
  const uint8_t* keycodes = (const uint8_t*)table;
  for (uint8_t i = 0; i < table_size_bytes; ++i) {
    if (target == pgm_read_byte(keycodes + i)) {
      // Xor (i ^ 1) the index to get the other element in the pair.
      return pgm_read_byte(keycodes + (i ^ 1));
    }
  }
  return KC_NO;
}

uint16_t get_alt_repeat_key_keycode(void) {
  uint16_t keycode = last_record.keycode;
  uint8_t mods = last_mods;

  // Call the user callback first to give it a chance to override the default
  // alternate key definitions that follow.
  uint16_t alt_keycode = get_alt_repeat_key_keycode_user(keycode, mods);

  if (alt_keycode != KC_TRNS) {
    return alt_keycode;
  }

  // Convert 8-bit mods to the 5-bit format used in keycodes. This is lossy: if
  // left and right handed mods were mixed, they all become right handed.
  mods = ((mods & 0xf0) ? /* set right hand bit */ 0x10 : 0)
         // Combine right and left hand mods.
         | (((mods >> 4) | mods) & 0xf);

  switch (keycode) {
    case QK_MODS ... QK_MODS_MAX:  // Unpack modifier + basic key.
      mods |= QK_MODS_GET_MODS(keycode);
      keycode = QK_MODS_GET_BASIC_KEYCODE(keycode);
      break;

#ifndef NO_ACTION_TAPPING
    case QK_MOD_TAP ... QK_MOD_TAP_MAX:
      keycode = QK_MOD_TAP_GET_TAP_KEYCODE(keycode);
      break;
#ifndef NO_ACTION_LAYER
    case QK_LAYER_TAP ... QK_LAYER_TAP_MAX:
      keycode = QK_LAYER_TAP_GET_TAP_KEYCODE(keycode);
      break;
#endif  // NO_ACTION_LAYER
#endif  // NO_ACTION_TAPPING

#ifdef SWAP_HANDS_ENABLE
    case QK_SWAP_HANDS ... QK_SWAP_HANDS_MAX:
      if (IS_SWAP_HANDS_KEYCODE(keycode)) {
        return KC_NO;
      }
      keycode = QK_SWAP_HANDS_GET_TAP_KEYCODE(keycode);
      break;
#endif  // SWAP_HANDS_ENABLE
  }

  if (IS_QK_BASIC(keycode)) {
    if ((mods & (MOD_LCTL | MOD_LALT | MOD_LGUI))) {
      // clang-format off
      static const uint8_t pairs[][2] PROGMEM = {
          {KC_F   , KC_B   },  // Forward / Backward.
          {KC_D   , KC_U   },  // Down / Up.
          {KC_N   , KC_P   },  // Next / Previous.
          {KC_A   , KC_E   },  // Home / End.
          {KC_O   , KC_I   },  // Vim jumplist Older / Newer.
      };
      // clang-format on
      alt_keycode = find_alt_keycode(pairs, sizeof(pairs), keycode);
    } else {
      // clang-format off
      static const uint8_t pairs[][2] PROGMEM = {
          {KC_J   , KC_K   },  // Down / Up.
          {KC_H   , KC_L   },  // Left / Right.
          {KC_W   , KC_B   },  // Forward / Backward by word.
          {KC_E   , KC_B   },  // Forward / Backward by word.
      };
      // clang-format on
      alt_keycode = find_alt_keycode(pairs, sizeof(pairs), keycode);
    }

    if (!alt_keycode) {
      // clang-format off
      static const uint8_t pairs[][2] PROGMEM = {
          {KC_LEFT, KC_RGHT},  // Left / Right Arrow.
          {KC_UP  , KC_DOWN},  // Up / Down Arrow.
          {KC_HOME, KC_END },  // Home / End.
          {KC_PGUP, KC_PGDN},  // Page Up / Page Down.
          {KC_BSPC, KC_DEL },  // Backspace / Delete.
          {KC_LBRC, KC_RBRC},  // Brackets [ ] and { }.
#ifdef EXTRAKEY_ENABLE
          {KC_WBAK, KC_WFWD},  // Browser Back / Forward.
          {KC_MNXT, KC_MPRV},  // Next / Previous Media Track.
          {KC_MFFD, KC_MRWD},  // Fast Forward / Rewind Media.
          {KC_VOLU, KC_VOLD},  // Volume Up / Down.
          {KC_BRIU, KC_BRID},  // Brightness Up / Down.
#endif  // EXTRAKEY_ENABLE
#ifdef MOUSEKEY_ENABLE
          {KC_MS_L, KC_MS_R},  // Mouse Cursor Left / Right.
          {KC_MS_U, KC_MS_D},  // Mouse Cursor Up / Down.
          {KC_WH_L, KC_WH_R},  // Mouse Wheel Left / Right.
          {KC_WH_U, KC_WH_D},  // Mouse Wheel Up / Down.
#endif  // MOUSEKEY_ENABLE
      };
      // clang-format on
      alt_keycode = find_alt_keycode(pairs, sizeof(pairs), keycode);
    }

    if (alt_keycode) {
      // Combine basic keycode with mods.
      return (mods << 8) | alt_keycode;
    }
  }

  return KC_NO;
}

void alt_repeat_key_invoke(const keyevent_t* event) {
  static keyrecord_t registered_record = {0};
  static int8_t registered_repeat_count = 0;
  if (processing_repeat_count) {
    return;
  }

  if (event->pressed) {
    registered_record = (keyrecord_t){
#ifndef NO_ACTION_TAPPING
        .tap.interrupted = false,
        .tap.count = 0,
#endif
        .keycode = get_alt_repeat_key_keycode(),
    };
  }

  if (!registered_record.keycode) {
    return;
  }

  if (event->pressed) {
    update_last_repeat_count(-1);
    registered_repeat_count = last_repeat_count;
  }

  registered_record.event = *event;
  processing_repeat_count = registered_repeat_count;
  process_record(&registered_record);
  processing_repeat_count = 0;
}

bool alt_repeat_key_register(void) {
  if (get_alt_repeat_key_keycode()) {
    alt_repeat_key_invoke(&MAKE_KEYEVENT(0, 0, true));
    return true;
  }
  return false;
}

bool alt_repeat_key_unregister(void) {
  if (get_alt_repeat_key_keycode()) {
    alt_repeat_key_invoke(&MAKE_KEYEVENT(0, 0, false));
    return true;
  }
  return false;
}

bool alt_repeat_key_tap(void) {
  if (get_alt_repeat_key_keycode()) {
    alt_repeat_key_register();
    wait_ms(TAP_CODE_DELAY);
    alt_repeat_key_unregister();
    return true;
  }
  return false;
}

__attribute__((weak)) uint16_t get_alt_repeat_key_keycode_user(uint16_t keycode,
                                                               uint8_t mods) {
  return KC_TRNS;
}

#endif  // NO_ALT_REPEAT_KEY

static bool get_repeat_key_eligible(uint16_t keycode, keyrecord_t* record) {
  switch (keycode) {
    // Ignore MO, TO, TG, TT, and TL layer switch keys.
    case QK_MOMENTARY ... QK_MOMENTARY_MAX:
    case QK_TO ... QK_TO_MAX:
    case QK_TOGGLE_LAYER ... QK_TOGGLE_LAYER_MAX:
    case QK_LAYER_TAP_TOGGLE ... QK_LAYER_TAP_TOGGLE_MAX:
    // Ignore mod keys.
    case KC_LCTL ... KC_RGUI:
    case KC_HYPR:
    case KC_MEH:
#ifndef NO_ACTION_ONESHOT  // Ignore one-shot keys.
    case QK_ONE_SHOT_LAYER ... QK_ONE_SHOT_LAYER_MAX:
    case QK_ONE_SHOT_MOD ... QK_ONE_SHOT_MOD_MAX:
#endif  // NO_ACTION_ONESHOT
#ifdef TRI_LAYER_ENABLE  // Ignore Tri Layer keys.
    case QK_TRI_LAYER_LOWER:
    case QK_TRI_LAYER_UPPER:
#endif  // TRI_LAYER_ENABLE
    case QK_LAYER_LOCK:
    case QK_REPEAT_KEY:
#ifndef NO_ALT_REPEAT_KEY
    case QK_ALT_REPEAT_KEY:
#endif  // NO_ALT_REPEAT_KEY
      return false;

    // Ignore hold events on tap-hold keys.
#ifndef NO_ACTION_TAPPING
    case QK_MOD_TAP ... QK_MOD_TAP_MAX:
#ifndef NO_ACTION_LAYER
    case QK_LAYER_TAP ... QK_LAYER_TAP_MAX:
#endif  // NO_ACTION_LAYER
      if (record->tap.count == 0) {
        return false;
      }
      break;
#endif  // NO_ACTION_TAPPING

#ifdef SWAP_HANDS_ENABLE
    case QK_SWAP_HANDS ... QK_SWAP_HANDS_MAX:
      if (IS_SWAP_HANDS_KEYCODE(keycode) || record->tap.count == 0) {
        return false;
      }
      break;
#endif  // SWAP_HANDS_ENABLE
  }

  return true;
}

static bool remember_last_key_wrapper(uint16_t keycode, keyrecord_t* record,
                                      uint8_t* remembered_mods) {
  return get_repeat_key_eligible(keycode, record) &&
         remember_last_key_user(keycode, record, remembered_mods);
}

bool process_record_repeat_key(uint16_t keycode, keyrecord_t* record) {
  if (get_repeat_key_count()) {
    return true;
  }

  if (keycode == QK_REPEAT_KEY) {
    repeat_key_invoke(&record->event);
    return false;
#ifndef NO_ALT_REPEAT_KEY
  } else if (keycode == QK_ALT_REPEAT_KEY) {
    alt_repeat_key_invoke(&record->event);
    return false;
#endif  // NO_ALT_REPEAT_KEY
  } else if (record->event.pressed) {
    uint8_t remembered_mods = get_mods() | get_weak_mods();
#ifndef NO_ACTION_ONESHOT
    remembered_mods |= get_oneshot_mods();
#endif  // NO_ACTION_ONESHOT

    if (remember_last_key_wrapper(keycode, record, &remembered_mods)) {
      set_last_record(keycode, record);
      set_last_mods(remembered_mods);
    }
  }

  return true;
}

__attribute__((weak)) bool remember_last_key_user(uint16_t keycode,
                                                  keyrecord_t* record,
                                                  uint8_t* remembered_mods) {
  return true;
}

#endif  // COMBO_ENABLE

