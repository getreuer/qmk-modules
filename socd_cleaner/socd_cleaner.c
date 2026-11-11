// Copyright 2024-2025 Google LLC
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
 * @file socd_cleaner.c
 * @brief SOCD Cleaner community module implementation
 *
 * For full documentation, see
 * <https://getreuer.info/posts/keyboards/socd-cleaner>
 */

#include "socd_cleaner.h"

#ifdef SOCD_CLEANER_RELEASE_DELAY_MIN_MS
#  include "timer.h"
#endif

ASSERT_COMMUNITY_MODULES_MIN_API_VERSION(1, 0, 0);

// Defined in introspection.c.
uint16_t socd_opposing_pairs_count(void);
socd_cleaner_t* socd_opposing_pairs_get(uint16_t index);

bool socd_cleaner_enabled = true;

#ifdef SOCD_CLEANER_RELEASE_DELAY_MIN_MS
#  ifdef SOCD_CLEANER_RELEASE_DELAY_MAX_MS
#    define SOCD_CLEANER_RANDOM_MASK \
      ((1UL << (sizeof(unsigned long) * 8 - \
                __builtin_clzl((unsigned long)(SOCD_CLEANER_RELEASE_DELAY_MAX_MS - \
                                               SOCD_CLEANER_RELEASE_DELAY_MIN_MS)))) - 1UL)

static uint16_t socd_prng_state = 0xA5C3U;

static uint16_t socd_prng(void) {
  socd_prng_state ^= socd_prng_state << 7;
  socd_prng_state ^= socd_prng_state >> 9;
  socd_prng_state ^= socd_prng_state << 8;
  return socd_prng_state;
}

static uint16_t random_release_delay(void) {
  uint16_t offset;
  do {
    offset = (uint16_t)(socd_prng() & SOCD_CLEANER_RANDOM_MASK);
  } while (offset > SOCD_CLEANER_RELEASE_DELAY_MAX_MS -
                        SOCD_CLEANER_RELEASE_DELAY_MIN_MS);

  return SOCD_CLEANER_RELEASE_DELAY_MIN_MS + offset;
}
#  endif

// All pending transitions are owned by this module. A count avoids scanning
// every pair in idle housekeeping without imposing a fixed pair-count limit.
static uint32_t delayed_release_pending_count;

static void cancel_delayed_release(socd_cleaner_t* state, uint8_t index) {
  if (state->delayed_release_pending[index]) {
    state->delayed_release_pending[index] = false;
    --delayed_release_pending_count;
  }
}

static void schedule_delayed_release(socd_cleaner_t* state, uint8_t index) {
  if (!state->delayed_release_pending[index]) {
    state->delayed_release_pending[index] = true;
    ++delayed_release_pending_count;
  }
  state->delayed_release_timer[index] = timer_read();
#  ifdef SOCD_CLEANER_RELEASE_DELAY_MAX_MS
  // Stir in the event timing at no additional timer-read cost.
  socd_prng_state =
      (socd_prng_state ^ state->delayed_release_timer[index]) | 1U;
  state->delayed_release_delay[index] = random_release_delay();
#  endif
}

static void cancel_all_delayed_releases(void) {
  for (uint16_t pair = 0; pair < socd_opposing_pairs_count(); ++pair) {
    socd_cleaner_t* state = socd_opposing_pairs_get(pair);
    if (!state) {
      continue;
    }
    state->delayed_release_pending[0] = false;
    state->delayed_release_pending[1] = false;
  }
  delayed_release_pending_count = 0;
}
#endif

#ifdef SOCD_CLEANER_MOUSEKEY_ENABLE
/**
 * Updates a key that SOCD Cleaner needs to synthesize. Returns true when the
 * key is a mouse cursor key and its mouse report is handled by QMK directly.
 */
static bool update_key(uint8_t keycode, bool press) {
  if (IS_MOUSEKEY_MOVE(keycode)) {
    if (press) {
      register_code(keycode);
    } else {
      unregister_code(keycode);
    }
    return true;
  }
#else
static void update_key(uint8_t keycode, bool press) {
#endif
  if (press) {
    add_key(keycode);
  } else {
    del_key(keycode);
  }
#ifdef SOCD_CLEANER_MOUSEKEY_ENABLE
  return false;
#endif
}

static bool process_opposing_pair(
    uint16_t keycode, keyrecord_t* record, socd_cleaner_t* state) {
  if (!state || !state->resolution ||
      (keycode != state->keys[0] && keycode != state->keys[1])) {
    return true;  // Quick return when disabled or on unrelated events.
  }

  // The current event corresponds to index `i`, 0 or 1, in the SOCD key pair.
  const uint8_t i = (keycode == state->keys[1]);
  const uint8_t opposing = i ^ 1;  // Index of the opposing key.

#ifdef SOCD_CLEANER_RELEASE_DELAY_MIN_MS
  // A physical event on this key supersedes any pending synthetic release.
  cancel_delayed_release(state, i);
#endif

  // Track which keys are physically held (vs. keys in the report).
  state->held[i] = record->event.pressed;

  // Perform SOCD resolution for events where the opposing key is held.
  if (state->held[opposing]) {
    switch (state->resolution) {
      case SOCD_CLEANER_LAST:  // Last input priority with reactivation.
#ifdef SOCD_CLEANER_RELEASE_DELAY_MIN_MS
        if (record->event.pressed) {
#  ifdef SOCD_CLEANER_MOUSEKEY_ENABLE
          if (IS_MOUSEKEY_MOVE(state->keys[opposing])) {
            // Mouse cursor directions always keep the original immediate SOCD
            // behavior. Release delay applies only to keyboard keys.
            update_key(state->keys[opposing], false);
          } else
#  endif
          {
            // Let the new key press continue immediately. Only the synthetic
            // release of the previously active keyboard key is delayed.
            schedule_delayed_release(state, opposing);
          }
        } else {
          // Reactivate the still physically held opposing key immediately. If
          // its delayed release has not fired yet, cancel that release first.
          cancel_delayed_release(state, opposing);
          update_key(state->keys[opposing], true);
        }
#else
        // If the current event is a press, then release the opposing key.
        // Otherwise if this is a release, then press the opposing key.
        update_key(state->keys[opposing], !state->held[i]);
#endif
        break;

      case SOCD_CLEANER_NEUTRAL:  // Neutral resolution.
        // Same logic as SOCD_CLEANER_LAST, but skip default handling so that
        // the current key has no effect while the opposing key is held.
#ifdef SOCD_CLEANER_MOUSEKEY_ENABLE
        // Mouse cursor registration sends its own mouse report. Keyboard keys
        // still need the report that default handling would normally send.
        if (!update_key(state->keys[opposing], !state->held[i])) {
          send_keyboard_report();
        }
#else
        update_key(state->keys[opposing], !state->held[i]);
        // Send updated report (normally, default handling would do this).
        send_keyboard_report();
#endif
        return false;  // Skip default handling.

      case SOCD_CLEANER_0_WINS:  // Key 0 wins.
      case SOCD_CLEANER_1_WINS:  // Key 1 wins.
        if (opposing == (state->resolution - SOCD_CLEANER_0_WINS)) {
          // The opposing key is the winner. The current key has no effect.
          return false;  // Skip default handling.
        } else {
          // The current key is the winner. Update logic is same as above.
          update_key(state->keys[opposing], !state->held[i]);
        }
        break;
    }
  }
  return true;  // Continue default handling to press/release current key.
}

bool process_record_socd_cleaner(uint16_t keycode, keyrecord_t* record) {
  switch (keycode) {
    case SOCDON:  // Turn SOCD Cleaner on.
      if (record->event.pressed) {
        socd_cleaner_enabled = true;
      }
      return false;
    case SOCDOFF:  // Turn SOCD Cleaner off.
      if (record->event.pressed) {
#ifdef SOCD_CLEANER_RELEASE_DELAY_MIN_MS
        cancel_all_delayed_releases();
#endif
        socd_cleaner_enabled = false;
      }
      return false;
    case SOCDTOG:  // Toggle SOCD Cleaner.
      if (record->event.pressed) {
#ifdef SOCD_CLEANER_RELEASE_DELAY_MIN_MS
        if (socd_cleaner_enabled) {
          cancel_all_delayed_releases();
        }
#endif
        socd_cleaner_enabled = !socd_cleaner_enabled;
      }
      return false;
  }

  if (socd_cleaner_enabled) {
    for (int i = 0; i < (int)socd_opposing_pairs_count(); ++i) {
      socd_cleaner_t* state = socd_opposing_pairs_get(i);
      if (!process_opposing_pair(keycode, record, state)) {
        return false;
      }
    }
  }
  return true;
}


#ifdef SOCD_CLEANER_RELEASE_DELAY_MIN_MS
void housekeeping_task_socd_cleaner(void) {
  if (!socd_cleaner_enabled || delayed_release_pending_count == 0) {
    return;
  }

  for (uint16_t pair = 0; pair < socd_opposing_pairs_count(); ++pair) {
    socd_cleaner_t* state = socd_opposing_pairs_get(pair);
    if (!state || state->resolution != SOCD_CLEANER_LAST) {
      continue;
    }

    for (uint8_t key = 0; key < 2; ++key) {
      if (!state->delayed_release_pending[key]) {
        continue;
      }

#  ifdef SOCD_CLEANER_RELEASE_DELAY_MAX_MS
      if (timer_elapsed(state->delayed_release_timer[key]) <
          state->delayed_release_delay[key]) {
#  else
      if (timer_elapsed(state->delayed_release_timer[key]) <
          SOCD_CLEANER_RELEASE_DELAY_MIN_MS) {
#  endif
        continue;
      }

      cancel_delayed_release(state, key);

      // A physical release cancels the pending synthetic release before this
      // task runs. Only release a key that is still physically held.
      if (!state->held[key]) {
        continue;
      }

      update_key(state->keys[key], false);
      send_keyboard_report();
    }
  }
}
#endif
