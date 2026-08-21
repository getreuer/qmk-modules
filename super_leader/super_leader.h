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
 * @file super_leader.h
 * @brief Super Leader: a suped-up Leader Key.
 *
 * Super Leader is like QMK's built-in Leader Key, but with two substantial
 * improvements:
 *
 * - More responsive: When a sequence is not a prefix of any other sequence, it
 *   resolves immediately when completed. No waiting for the timeout.
 *
 * - Sequence definitions are more concise, typically one line per sequence.
 *   Built-in helpers enable sequences to result in sending a string or Unicode
 *   string or calling a user-defined function.
 *
 * To use this module, add it to your keymap.json:
 *
 *     {
 *       "modules": ["getreuer/super_leader"]
 *     }
 *
 * In your keymap.c file, add a LEADER key somewhere in your layout. Then in
 * your keymap folder, add a file super_leader.def to define one or more
 * leader sequences like:
 *
 *     SEQ_KEY(v,   (KC_V),             C(KC_V))
 *     SEQ_KEY(dfu, (KC_D, KC_F, KC_U), QK_BOOT)
 *     SEQ_STR(me,  (KC_M, KC_E),       "my@email.com")
 *
 * Then, for instance, tapping the keys "LEADER, M, E" in sequence produces
 * "my@email.com".
 *
 * For full documentation, see
 * <https://getreuer.info/posts/keyboards/super-leader>
 */

#pragma once

#include "quantum.h"

#ifdef __cplusplus
extern "C" {
#endif

// Getting convenient sequence definitions takes some macro shenanigans:
// * The "CHOOSER" macro selects between 1-arg and 2-arg definitions of a macro
//   based on the number of input args, emulating function overloading.
// * The "ARRAY" macro defines a 0-terminated array from the given sequence.

#define SUPER_LEADER_CHOOSER_(_1, _2, NAME, ...) NAME
#define SUPER_LEADER_ARRAY1_(...) {__VA_ARGS__ __VA_OPT__(, ) 0}
#define SUPER_LEADER_ARRAY_(keys_) SUPER_LEADER_ARRAY1_ keys_

#define SUPER_LEADER_KEY(kc_)                                \
  (super_leader_output_t) {                                  \
    NULL, (super_leader_output_param_t) { .keycode = (kc_) } \
  }
#define SUPER_LEADER_FUN(...)                             \
  SUPER_LEADER_CHOOSER_(__VA_ARGS__, SUPER_LEADER_FUN_2_, \
                        SUPER_LEADER_FUN_1_)(__VA_ARGS__)
#define SUPER_LEADER_FUN_1_(fn_) SUPER_LEADER_FUN_2_(fn_, NULL)
#define SUPER_LEADER_FUN_2_(fn_, data_)                                  \
  (super_leader_output_t) {                                              \
    (fn_), (super_leader_output_param_t) { .user_data = (void*)(data_) } \
  }
#define SUPER_LEADER_STR(str)                                 \
  (super_leader_output_t) {                                   \
    super_leader_send_string, (super_leader_output_param_t) { \
      .user_data = (void*)PSTR(str)                           \
    }                                                         \
  }
#define SUPER_LEADER_UNI(str)                                         \
  (super_leader_output_t) {                                           \
    super_leader_send_unicode_string, (super_leader_output_param_t) { \
      .user_data = (void*)(str)                                       \
    }                                                                 \
  }

#define SUPER_LEADER_SEQ_STARTS_WITH(keys_, partial_)                        \
  super_leader_seq_starts_with((const uint16_t[])SUPER_LEADER_ARRAY_(keys_), \
                               (partial_))

enum { SUPER_LEADER_END = KC_NO };

typedef union {
  uint16_t keycode;
  void* user_data;
} super_leader_output_param_t;

typedef struct {
  void (*fn)(void*);
  super_leader_output_param_t param;
} super_leader_output_t;

typedef struct {
  const uint16_t* keys;
  super_leader_output_t output;
} super_leader_sequence_t;

extern super_leader_sequence_t super_leader_sequences[];

bool super_leader_start_internal(keyrecord_t* record);
bool super_leader_add_internal(uint16_t keycode, keyrecord_t* record);

// Optional user callbacks.
void super_leader_start_user(void);
void super_leader_end_user(bool successful_match);
void super_leader_add_user(const uint16_t* seq, uint8_t num_seq, bool* partial);

/** Whether a leader sequence is active. */
bool super_leader_sequence_active(void);

/**
 * Taps the LEADER key, beginning the leader sequence. Or if a sequence is
 * already active, LEADER is added to the sequence buffer.
 */
static inline void super_leader_start(void) {
  (void)super_leader_start_internal(NULL);
}

/** Cancels the leader sequence, if active. */
void super_leader_cancel(void);

/** Adds a key to the sequence buffer. */
static inline void super_leader_add(uint16_t keycode) {
  (void)super_leader_add_internal(keycode, NULL);
}

/** Resets the leader sequence timer. */
void super_leader_reset_timer(void);

/**
 * Indicates that a sequence has matched and sets its output.
 *
 * Use one of the SUPER_LEADER_*() macros to construct the argument:
 *
 *     super_leader_set_match(SUPER_LEADER_KEY(kc));
 *     super_leader_set_match(SUPER_LEADER_STR("string"));
 *     super_leader_set_match(SUPER_LEADER_UNI("unicode"));
 *     super_leader_set_match(SUPER_LEADER_FUN(fun, user_data));
 */
void super_leader_set_match(super_leader_output_t output);

// Used internally by Super Leader macros.
void super_leader_send_string(void* user_data);
void super_leader_send_unicode_string(void* user_data);
bool super_leader_seq_starts_with(const uint16_t* keys, bool* partial);

#ifdef __cplusplus
}
#endif
