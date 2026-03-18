// Copyright 2026 Google LLC
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
 * @file cyclotab.c
 * @brief Cyclotab community module implementation
 *
 * For full documentation, see
 * <https://getreuer.info/posts/keyboards/cyclotab>
 */

#include "cyclotab.h"

ASSERT_COMMUNITY_MODULES_MIN_API_VERSION(1, 0, 0);

#ifndef CYCLOTAB_KEYS
#  define CYCLOTAB_KEYS A(KC_TAB)
#endif  // CYCLOTAB_KEYS
#ifndef CYCLOTAB_TIMEOUT
#  define CYCLOTAB_TIMEOUT 1000  // 1 second.
#endif  // CYCLOTAB_TIMEOUT

static const uint16_t cyclotab_keys[] = {CYCLOTAB_KEYS};
#define NUM_CYCLOTAB_KEYS (ARRAY_SIZE(cyclotab_keys))
       
static uint16_t release_timer = 0;
static uint16_t active_key = KC_NO;
static uint8_t active_mods = 0;
static bool shift_held = false;

static bool is_trigger_keycode(uint16_t cyclotab_key, uint16_t keycode) {
  return IS_QK_MODS(cyclotab_key) &&
    (keycode == cyclotab_key || keycode == S(cyclotab_key));
}

static void update_release_timer(keyrecord_t* record) {
  uint16_t timeout = 0;
  if (shift_held || record->event.pressed || 
      (timeout = cyclotab_timeout(active_key)) == 0) {
    release_timer = 0;  // Pause timer.
  } else {
    release_timer = (record->event.time + timeout) | 1;
  }
}

static void release_active(void) {
  if (active_key != KC_NO) {
#ifdef REPEAT_KEY_ENABLED
    if (get_last_keycode() != active_key) {
      set_last_keycode(active_key);
      set_last_mods(0);
    }
#endif  // REPEAT_KEY_ENABLED
    unregister_mods(active_mods);
    active_key = KC_NO;
    release_timer = 0;
    shift_held = false;
  }
}

bool process_record_cyclotab(uint16_t keycode, keyrecord_t* record) {
  if (active_key != KC_NO) {
    bool stay_active = false;

#ifndef NO_ACTION_TAPPING
    switch (keycode) {  // Parse tap-hold keys.
      case QK_MOD_TAP ... QK_MOD_TAP_MAX:
        if (record->tap.count > 0) {
          keycode = QK_MOD_TAP_GET_TAP_KEYCODE(keycode);
        } else if ((QK_MOD_TAP_GET_MODS(keycode) & (MOD_HYPR)) == MOD_LSFT) {
          keycode = KC_LSFT;
        }
        break;

      case QK_LAYER_TAP ... QK_LAYER_TAP_MAX:
        if (record->tap.count > 0) {
          keycode = QK_LAYER_TAP_GET_TAP_KEYCODE(keycode);
        }
        break;
    }
#endif  // NO_ACTION_TAPPING

    switch (keycode) {
      case KC_LSFT:
      case KC_RSFT:
#ifndef NO_ACTION_ONESHOT
      case OSM(MOD_LSFT):
      case OSM(MOD_RSFT):
#endif  // NO_ACTION_ONESHOT
        shift_held = record->event.pressed;
        // Fallthrough intended.

      case KC_RGHT:
      case KC_LEFT:
      case KC_DOWN:
      case KC_UP:
        stay_active = true;
        break;

      case KC_ESC:
        tap_code(KC_ESC);
        break;
    }

    if (stay_active || is_trigger_keycode(cyclotab_active_key(), keycode)) {
      update_release_timer(record);
      return true;
    }

    release_active();  // Release mods when another key is pressed.
    // If this event is a key press, skip further handling to "consume" the key.
    return !record->event.pressed;
  } 

  if (record->event.pressed) {
    for (int8_t i = 0; i < NUM_CYCLOTAB_KEYS; ++i) {
      const uint16_t cyclotab_key = cyclotab_keys[i];
      if (is_trigger_keycode(cyclotab_key, keycode)) {
        active_key = cyclotab_key;
        uint8_t mods = mod_config(QK_MODS_GET_MODS(active_key));
        active_mods = ((mods & 0x10) == 0) ? mods : (mods << 4);
        register_mods(active_mods);  // Activate mods.
        break;
      }
    }
  }

  return true;
}

void housekeeping_task_cyclotab(void) {
  if (release_timer && timer_expired(timer_read(), release_timer)) {
    release_active();  // Release mods on timeout.
  }
}

uint16_t cyclotab_active_key(void) {
  return active_key;
}

__attribute__((weak)) uint16_t cyclotab_timeout(uint16_t keycode) {
  return (CYCLOTAB_TIMEOUT);
}

