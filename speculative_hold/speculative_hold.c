// Copyright 2025 Google LLC
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
 * @file speculative_hold.c
 * @brief Speculative Hold module implementation
 *
 * For full documentation, see
 * <https://getreuer.info/posts/keyboards/speculative-hold>
 */

#include "speculative_hold.h"

#pragma message \
    "Speculative Hold is now a core QMK feature! To use it, update your QMK set up and see https://docs.qmk.fm/tap_hold#speculative-hold"

ASSERT_COMMUNITY_MODULES_MIN_API_VERSION(1, 0, 0);

// Events bypass Speculative Hold when there are unsettled LT keys in
// action_tapping's waiting_queue.
static uint16_t settle_timer = 0;

typedef struct {
  keypos_t key;
  uint8_t mods;
} held_key_t;
#define HELD_KEYS_SIZE 8
static held_key_t held_keys[HELD_KEYS_SIZE] = {};
static uint8_t num_held_keys = 0;
static uint8_t speculative_mods = 0;

static void held_keys_add(keypos_t key, uint8_t mods) {
  if (num_held_keys >= HELD_KEYS_SIZE) {  // Overflow!
    clear_keyboard();
    num_held_keys = 0;
  }

  held_keys[num_held_keys] = (held_key_t){
      .key = key,
      .mods = mods,
  };
  ++num_held_keys;
}

static int8_t held_keys_find(keypos_t key) {
  for (int8_t i = 0; i < num_held_keys; ++i) {
    if (KEYEQ(held_keys[i].key, key)) {
      return i;
    }
  }
  return -1;
}

static void held_keys_del_index(int8_t i) {
  if (i < num_held_keys) {
    --num_held_keys;
    for (int8_t j = i; j < num_held_keys; ++j) {
      held_keys[j] = held_keys[j + 1];
    }
  }
}

#if defined(SPECULATIVE_HOLD_DEBUG) && !defined(NO_DEBUG)
static void held_keys_debug(void) {
  if (debug_enable) {
    xprintf("speculative_hold: held_keys = { ");
    for (int8_t i = 0; i < num_held_keys; ++i) {
      const keypos_t key = held_keys[i].key;
      xprintf("%02x%02x%c ", key.row, key.col, held_keys[i].mods ? ' ' : '*');
    }
    xprintf("}\n");
  }
}
#else
#define held_keys_debug()
#endif

#ifdef COMBO_ENABLE
#include "keymap_introspection.h"

// Check whether `keycode` is part of combo.
static bool is_combo_key(uint16_t keycode) {
  for (uint16_t i = 0; i < combo_count(); ++i) {
    const uint16_t* keys = combo_get(i)->keys;
    uint16_t key;
    do {
      key = pgm_read_word(keys++);
      if (key == keycode) {
        return true;
      }
    } while (key != COMBO_END);
  }
  return false;
}
#else
#define is_combo_key(keycode) false
#endif  // COMBO_ENABLE

void housekeeping_task_speculative_hold(void) {
  if (settle_timer && timer_expired(timer_read(), settle_timer)) {
    settle_timer = 0;
  }
}

static uint8_t unpack_mods(uint16_t keycode) {
  const uint8_t mods5 = QK_MOD_TAP_GET_MODS(keycode);
  return (mods5 & 0x10) != 0 ? (mods5 << 4) : mods5;
}

// This hook is called for every key event, *before* QMK core processes it.
bool pre_process_record_speculative_hold(uint16_t keycode,
                                         keyrecord_t* record) {
  // Ignore events other than MT and LT press events.
  if (!(record->event.pressed &&
        (IS_QK_MOD_TAP(keycode) || IS_QK_LAYER_TAP(keycode)))) {
    return true;
  }

  if (IS_QK_LAYER_TAP(keycode)) {
    // If this is an LT key, track when it will settle.
    const uint16_t term = GET_TAPPING_TERM(keycode, record);
    const uint16_t now = timer_read();
    if (!settle_timer || term > TIMER_DIFF_16(settle_timer, now)) {
      settle_timer = (now + term) | 1;
    }
  } else if (!is_combo_key(keycode)) {
    // Otherwise if this is an MT key and not part of a combo...
    const keypos_t key = record->event.key;
    const uint8_t mods = unpack_mods(keycode);

    if (!settle_timer && (~(get_mods() | speculative_mods) & mods) != 0 &&
        get_speculative_hold(keycode, record)) {
      // If there are no unsettled LTs, the mods are not already applied,
      // and get_speculative_hold() is true, then apply mods speculatively.
#ifdef SPECULATIVE_HOLD_DEBUG
      dprintf("speculative_hold: %02x%02x: press (%s).\n", key.row, key.col,
              get_keycode_string(keycode));
#endif  // SPECULATIVE_HOLD_DEBUG
      held_keys_add(key, mods);
      held_keys_debug();

      speculative_mods |= mods;
      add_weak_mods(speculative_mods);
      send_keyboard_report();
    } else if (num_held_keys > 0) {
      held_keys_add(key, 0);
      held_keys_debug();
    }
  }

  return true;
}

// This hook is called for every key event, *after* the core tapping processing,
// but before most other feature handlers.
bool process_record_speculative_hold(uint16_t keycode, keyrecord_t* record) {
  if (num_held_keys == 0 || !record->event.pressed) {
    return true;
  }

  add_weak_mods(speculative_mods);
  int8_t i = held_keys_find(record->event.key);

  if (IS_QK_MOD_TAP(keycode) && record->tap.count == 0) {  // MT hold press.
    if (i != -1) {
      if (held_keys[i].mods) {
#ifdef SPECULATIVE_HOLD_DEBUG
        dprintf("speculative_hold: %02x%02x: settled.\n", held_keys[i].key.row,
                held_keys[i].key.col);
#endif  // SPECULATIVE_HOLD_DEBUG
      }
      const uint8_t cleared_mods = held_keys[i].mods;
      held_keys_del_index(i);
      held_keys_debug();

      // Clear the speculatively-held mod. The hold press handling will apply
      // the mod again.
      speculative_mods &= ~cleared_mods;
      del_weak_mods(cleared_mods);
    }
  } else {  // Tap press event; cancel speculatively-held mod.
    uint8_t cleared_mods = 0;

    if (i == -1) {
      i = 0;
    }

    // Clear mods for the ith key and all keys that follow.
    for (int8_t j = num_held_keys - 1; j >= i; --j) {
#ifdef SPECULATIVE_HOLD_DEBUG
      if (held_keys[j].mods) {
        dprintf("speculative_hold: %02x%02x: cancel.\n", held_keys[j].key.row,
                held_keys[j].key.col);
      }
#endif  // SPECULATIVE_HOLD_DEBUG
      cleared_mods |= held_keys[j].mods;
    }

    num_held_keys = i;
    held_keys_debug();

    if (cleared_mods) {
#ifdef REPEAT_KEY_ENABLE
      // Repeat Key's process record handler runs before modules. This means the
      // current tap press event was already recorded while the speculative mods
      // were still applied. We correct for that here.
      set_last_mods(get_last_mods() & ~cleared_mods);
#endif  // REPEAT_KEY_ENABLE
#ifdef DUMMY_MOD_NEUTRALIZER_KEYCODE
      neutralize_flashing_modifiers(cleared_mods);
#endif  // DUMMY_MOD_NEUTRALIZER_KEYCODE

      speculative_mods &= ~cleared_mods;
      del_weak_mods(cleared_mods);
      send_keyboard_report();
    }
  }

  return true;
}

__attribute__((weak)) bool get_speculative_hold(uint16_t keycode,
                                                keyrecord_t* record) {
  return (QK_MOD_TAP_GET_MODS(keycode) & (MOD_LALT | MOD_LGUI)) == 0;
}
