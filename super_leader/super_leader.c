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
 * @file super_leader.c
 * @brief Super Leader community module implementation
 *
 * For full documentation, see
 * <https://getreuer.info/posts/keyboards/super-leader>
 */

#include "super_leader.h"

ASSERT_COMMUNITY_MODULES_MIN_API_VERSION(1, 0, 0);

#if !defined(COMBO_ENABLE) && !defined(REPEAT_KEY_ENABLE)
#error \
    "super_leader: Please enable Combos (COMBO_ENABLE = true) or Repeat Key (REPEAT_KEY_ENABLE = yes), or both, in rules.mk."
#else

#ifndef SUPER_LEADER_TIMEOUT
#define SUPER_LEADER_TIMEOUT 1000
#endif  // SUPER_LEADER_TIMEOUT
#ifndef SUPER_LEADER_MAX_LENGTH
#define SUPER_LEADER_MAX_LENGTH 5
#endif  // SUPER_LEADER_MAX_LENGTH

void super_leader_send_string(void* user_data) {
  send_string_P((const char*)user_data);
}

#ifdef UNICODE_COMMON_ENABLE
void super_leader_send_unicode_string(void* user_data) {
  send_unicode_string((const char*)user_data);
}
#endif

static struct {
  keyrecord_t record;
  super_leader_output_t match;
  uint16_t timer;
  uint16_t seq[SUPER_LEADER_MAX_LENGTH];
  uint8_t num_seq;
  uint8_t num_matched;
  uint8_t state;
} leader = {0};

enum {
  STATE_OFF,        // Leader inactive.
  STATE_RECURSING,  // Inside process_record re-entry guard.
  STATE_ACTIVE,     // Accepting sequence keys.
  STATE_KEY_HELD,   // Matched; output key physically held.
};

// Defined in introspection.c.
uint16_t super_leader_sequence_count(void);
super_leader_sequence_t* super_leader_sequence_get(uint16_t index);

static bool have_full_match(void) {
  return leader.match.fn != NULL || leader.match.param.keycode != KC_NO;
}

static void process_output_event(uint16_t keycode, bool pressed) {
  dprintf("super_leader: Processing %s of %s\n", pressed ? "press" : "release",
          get_keycode_string(keycode));

  switch (keycode) {
    case QK_ONE_SHOT_MOD ... QK_ONE_SHOT_MOD_MAX:
      if (pressed) {
        uint8_t mods = QK_ONE_SHOT_MOD_GET_MODS(keycode);
        if ((mods & 0x10) != 0) {
          mods <<= 4;
        }  // Expand 5-bit mods to 8 bits.
        add_oneshot_mods(mods);
      }
      leader.state = STATE_OFF;
      return;

    case QK_ONE_SHOT_LAYER ... QK_ONE_SHOT_LAYER_MAX:
      if (!pressed) {
        // On release, set ONESHOT_OTHER_KEY_PRESSED (not ONESHOT_START) so that
        // the layer is cleared on the next keypress.
        set_oneshot_layer(QK_ONE_SHOT_LAYER_GET_LAYER(keycode),
                          ONESHOT_OTHER_KEY_PRESSED);
      }
      break;

    default:
      leader.record.keycode = keycode;
      leader.state = STATE_RECURSING;

      leader.record.event.pressed = pressed;
      leader.record.event.time = timer_read();
      process_record(&leader.record);
      break;
  }
  leader.state = pressed ? STATE_KEY_HELD : STATE_OFF;
}

static void process_output_tap(uint16_t keycode) {
  process_output_event(keycode, true);
#if TAP_CODE_DELAY > 0
  wait_ms(TAP_CODE_DELAY);
#endif  // TAP_CODE_DELAY > 0
  process_output_event(keycode, false);
}

static void set_finished_state(void) {
  const bool successful_match = have_full_match();
#ifndef NO_DEBUG
  if (debug_enable && successful_match) {
    const char* output = (leader.match.fn != NULL)
                             ? "callback"
                             : get_keycode_string(leader.match.param.keycode);
    dprintf("super_leader: Finished, output: %s\n", output);
  }
#endif  // NO_DEBUG

  leader.state = STATE_OFF;
  super_leader_end_user(successful_match);
}

static void super_leader_finish(void) {
  if (leader.state != STATE_ACTIVE) {
    return;
  }

  const uint8_t saved_mods = get_mods();
#ifndef NO_ACTION_ONESHOT
  const uint8_t saved_oneshot_mods = get_oneshot_mods();
#endif  // NO_ACTION_ONESHOT
  if (leader.num_seq > leader.num_matched) {
    clear_mods();
#ifndef NO_ACTION_ONESHOT
    clear_oneshot_mods();
#endif  // NO_ACTION_ONESHOT
    send_keyboard_report();
  }

  if (have_full_match()) {
    set_finished_state();

    if (leader.match.fn == NULL) {
      process_output_tap(leader.match.param.keycode);
    } else {
      leader.match.fn(leader.match.param.user_data);
    }
  } else {
    super_leader_cancel();
  }

  for (uint8_t i = leader.num_matched; i < leader.num_seq; ++i) {
    process_output_tap(leader.seq[i]);
  }

  if (leader.num_seq > leader.num_matched) {
    set_mods(saved_mods);
#ifndef NO_ACTION_ONESHOT
    set_oneshot_mods(saved_oneshot_mods);
#endif  // NO_ACTION_ONESHOT
    send_keyboard_report();
  }
}

// Returns true if this key event should be ignored by the
// process_record_super_leader() handler.
static bool is_ignored_key(uint16_t keycode, keyrecord_t* record) {
  switch (keycode) {
    case QK_MOMENTARY ... QK_MOMENTARY_MAX:
    case QK_TO ... QK_TO_MAX:
    case QK_TOGGLE_LAYER ... QK_TOGGLE_LAYER_MAX:
    case QK_LAYER_TAP_TOGGLE ... QK_LAYER_TAP_TOGGLE_MAX:
#ifndef NO_ACTION_TAPPING
    case QK_LAYER_TAP ... QK_LAYER_TAP_MAX:
    case QK_MOD_TAP ... QK_MOD_TAP_MAX:
      return record->tap.count == 0;
#endif  // NO_ACTION_TAPPING
#ifndef NO_ACTION_ONESHOT
    case QK_ONE_SHOT_LAYER ... QK_ONE_SHOT_LAYER_MAX:
    case QK_ONE_SHOT_MOD ... QK_ONE_SHOT_MOD_MAX:
#endif  // NO_ACTION_ONESHOT
    case MODIFIER_KEYCODE_RANGE:
      return true;
  }
  return false;
}

static bool match_sequence(const uint16_t* keys) {
  for (uint8_t i = 0; i < leader.num_seq; ++i) {
    if (leader.seq[i] != pgm_read_word(&keys[i])) {
      return false;
    }
  }
  return true;
}

void super_leader_set_match(super_leader_output_t output) {
  leader.match = output;
  leader.num_matched = leader.num_seq;
}

bool super_leader_seq_starts_with(const uint16_t* keys, bool* partial) {
  for (int8_t i = 0; i < leader.num_seq; ++i) {
    if (leader.seq[i] != keys[i]) {
      return keys[i] == KC_NO;
    }
  }
  *partial = true;
  return false;
}

// Appends a key to the buffer. Returns true on success, false if full.
static bool append_to_buffer(uint16_t keycode, keyrecord_t* record) {
  if (leader.num_seq >= SUPER_LEADER_MAX_LENGTH) {
    super_leader_cancel();
    return false;
  }

  leader.seq[leader.num_seq++] = keycode;
#ifndef NO_DEBUG
  if (debug_enable) {
    dprintf("super_leader: seq = { ");
    for (int8_t i = 0; i < leader.num_seq; ++i) {
      dprintf("%s ", get_keycode_string(leader.seq[i]));
    }
    dprintf("}\n");
  }
#endif  // NO_DEBUG

  return true;
}

// Searches for a matching leader sequence. Returns whether default handling
// should continue for this key event.
static bool find_sequence(uint16_t keycode, keyrecord_t* record) {
  bool partial_match = false;
  super_leader_reset_timer();

  for (uint16_t i = 0; i < super_leader_sequence_count(); ++i) {
    const super_leader_sequence_t* entry = super_leader_sequence_get(i);
    if (match_sequence(entry->keys)) {
      if (leader.num_seq == SUPER_LEADER_MAX_LENGTH ||
          pgm_read_word(&entry->keys[leader.num_seq]) == 0) {  // Full match.
        super_leader_set_match(entry->output);
      } else {  // Partial match.
        partial_match = true;
      }
    }
  }

  bool user_partial = partial_match;
  super_leader_add_user(leader.seq, leader.num_seq, &user_partial);
  partial_match |= user_partial;

  if (leader.num_matched == leader.num_seq && record) {
    leader.record.event.key = record->event.key;
  }

  const uint8_t active_mods = (get_mods()
#ifndef NO_ACTION_ONESHOT
                               | get_oneshot_mods()
#endif  // NO_ACTION_ONESHOT
  );
  if (!partial_match || active_mods) {
    if (leader.num_matched == leader.num_seq) {
      // Current key completed a sequence unambiguously.
      if (leader.match.fn == NULL && record) {
        const uint16_t keycode = leader.match.param.keycode;
        set_finished_state();
        if (IS_QK_ONE_SHOT_LAYER(keycode)) {
          // Defer OSL activation to key release.
          leader.state = STATE_KEY_HELD;
        } else {
          process_output_event(keycode, true);
        }
      } else {
        super_leader_finish();
      }
    } else {  // Current key follows a (possibly failed) leader sequence.
      const uint8_t saved_mods = get_mods();
#ifndef NO_ACTION_ONESHOT
      const uint8_t saved_oneshot_mods = get_oneshot_mods();
#endif  // NO_ACTION_ONESHOT
      clear_mods();
#ifndef NO_ACTION_ONESHOT
      clear_oneshot_mods();
#endif  // NO_ACTION_ONESHOT
      send_keyboard_report();

      --leader.num_seq;
      super_leader_finish();

      set_mods(saved_mods);
#ifndef NO_ACTION_ONESHOT
      set_oneshot_mods(saved_oneshot_mods);
#endif  // NO_ACTION_ONESHOT
      send_keyboard_report();

      if (!record) {
        process_output_tap(keycode);
      }
      return true;
    }
  }

  return false;
}

bool super_leader_start_internal(keyrecord_t* record) {
  if (leader.state != STATE_ACTIVE) {
    super_leader_cancel();
    leader.state = STATE_ACTIVE;
    leader.num_seq = 0;
    leader.num_matched = 0;
    leader.record.event = MAKE_KEYEVENT(0, 0, true);
    leader.record.keycode = KC_NO;
    leader.match.fn = NULL;
    leader.match.param.keycode = KC_NO;
    memset(leader.seq, 0, SUPER_LEADER_MAX_LENGTH * sizeof(uint16_t));
    dprintf("super_leader: Started.\n");
    super_leader_start_user();
  } else {
    append_to_buffer(LEADER, record);
  }

  return find_sequence(LEADER, record);
}

void super_leader_cancel(void) {
  if (leader.state != STATE_OFF) {
    dprintf("super_leader: Canceled.\n");
    if (leader.state == STATE_KEY_HELD) {
      clear_keyboard();
    }
    leader.state = STATE_OFF;
    super_leader_end_user(false);
  }
}

void super_leader_reset_timer(void) {
  leader.timer = timer_read() + SUPER_LEADER_TIMEOUT;
}

bool super_leader_add_internal(uint16_t keycode, keyrecord_t* record) {
  return leader.state != STATE_ACTIVE || !append_to_buffer(keycode, record) ||
         find_sequence(keycode, record);
}

bool super_leader_sequence_active(void) { return leader.state == STATE_ACTIVE; }

bool process_record_super_leader(uint16_t keycode, keyrecord_t* record) {
  if (leader.state == STATE_RECURSING || is_ignored_key(keycode, record)) {
    return true;
  } else if (!record->event.pressed) {
    if (leader.state == STATE_KEY_HELD &&
        KEYEQ(record->event.key, leader.record.event.key)) {
      process_output_event(leader.match.param.keycode, false);
      return false;
    }
    return keycode != LEADER;
  } else if (leader.state != STATE_ACTIVE) {
    if (keycode == LEADER) {
      return super_leader_start_internal(record);
    }
    return true;
  }

#ifndef SUPER_LEADER_STRICT_KEY_PROCESSING
  keycode = get_tap_keycode(keycode);
#endif  // SUPER_LEADER_STRICT_KEY_PROCESSING
  return super_leader_add_internal(keycode, record);
}

void housekeeping_task_super_leader(void) {
  if (leader.state == STATE_ACTIVE &&
#ifdef SUPER_LEADER_NO_INIT_TIMEOUT
      leader.num_seq > 0 &&
#endif  // SUPER_LEADER_NO_INIT_TIMEOUT
      timer_expired(timer_read(), leader.timer)) {
    dprintf("super_leader: Timed out.\n");
    super_leader_finish();
  }
}

__attribute__((weak)) void super_leader_start_user(void) {}
__attribute__((weak)) void super_leader_end_user(bool successful_match) {}
__attribute__((weak)) void super_leader_add_user(const uint16_t* seq,
                                                 uint8_t num_seq,
                                                 bool* partial) {}

#endif  // !defined(COMBO_ENABLE) && !defined(REPEAT_KEY_ENABLE)
