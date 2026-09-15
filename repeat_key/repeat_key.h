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
 * @file repeat_key.h
 * @brief Repeat Key community module: repeat the last pressed key.
 *
 * Overview
 * --------
 *
 * @note Repeat Key has evolved into core QMK feature Repeat Key! See
 * <https://docs.qmk.fm/features/repeat_key>
 *
 * Repeat Key performs the action of the last pressed key. Tapping Repeat Key
 * after tapping Z types another "z." Repeat Key remembers the modifiers that
 * were active with the last key press and stacks them on top of active mods.
 *
 * Also included is Alternate Repeat Key, performing the "alternate" if there
 * is one for the last key (e.g. Page Down -> Page Up, Left -> Right, etc.).
 *
 * For full documentation, see
 * <https://getreuer.info/posts/keyboards/repeat-key>
 */

#pragma once

#include "quantum.h"

#ifndef REPEAT_KEY_ENABLE
#ifdef __cplusplus
extern "C" {
#endif

/** Resets repeat key state. */
void reset_repeat_key_state(void);

/** Keycode of the last key. */
uint16_t get_last_keycode(void);

/** Mods that were active with the last key. */
uint8_t get_last_mods(void);

/** Sets the last keycode. */
void set_last_keycode(uint16_t keycode);

/** Sets the last mods. */
void set_last_mods(uint8_t mods);

/** Gets the record for the last key. */
const keyrecord_t* get_last_record(void);

/** Sets keycode and record info for the last key. */
void set_last_record(uint16_t keycode, keyrecord_t* record);

/**
 * @brief Signed count of times the key has been repeated or alternate repeated.
 *
 * Nonzero only while a repeated or alternate-repeated key is being processed:
 * positive (1..127) for repeat, negative (-1..-127) for alternate repeat.
 */
int8_t get_repeat_key_count(void);

/** Calls `process_record()` on a generated record repeating the last key. */
void repeat_key_invoke(const keyevent_t* event);

/** Registers (presses down) the Repeat Key. */
void repeat_key_register(void);

/** Unregisters (releases) the Repeat Key. */
void repeat_key_unregister(void);

/** Taps the Repeat Key with a delay of `TAP_CODE_DELAY`. */
void repeat_key_tap(void);

#  ifndef NO_ALT_REPEAT_KEY

/** Keycode to be used for alternate repeating. */
uint16_t get_alt_repeat_key_keycode(void);

/** Calls `process_record()` to alternate repeat the last key. */
void alt_repeat_key_invoke(const keyevent_t* event);

/** Registers (presses down) the Alternate Repeat Key. */
bool alt_repeat_key_register(void);

/** Unregisters (releases) the Alternate Repeat Key. */
bool alt_repeat_key_unregister(void);

/** Taps the Alternate Repeat Key with a delay of `TAP_CODE_DELAY`. */
bool alt_repeat_key_tap(void);

/**
 * @brief Optional user callback to define additional alternate keys.
 *
 * @param keycode Keycode of the last key.
 * @param mods    Modifiers active with the last key.
 * @return Alternate keycode, or KC_TRNS to defer to default rules.
 */
uint16_t get_alt_repeat_key_keycode_user(uint16_t keycode, uint8_t mods);

#  endif  // NO_ALT_REPEAT_KEY

/**
 * @brief Optional user callback defining which keys are remembered.
 *
 * @param keycode          Keycode that was just pressed.
 * @param record           keyrecord_t structure.
 * @param remembered_mods  Mods that will be remembered with this key.
 * @return true if key is remembered, false if ignored.
 */
bool remember_last_key_user(uint16_t keycode, keyrecord_t* record,
                            uint8_t* remembered_mods);

#ifdef __cplusplus
}
#endif
#endif  // REPEAT_KEY_ENABLE
