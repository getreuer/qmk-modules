// Copyright 2021-2026 Google LLC
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
 * @file caps_word.h
 * @brief Caps Word, a modern alternative to Caps Lock
 *
 * Overview
 * --------
 *
 * @note Caps Word has evolved into core QMK feature Caps Word! See
 * <https://docs.qmk.fm/features/caps_word>
 *
 * This community module implements "Caps Word", which is like conventional Caps
 * Lock, but automatically disables itself at the end of the word. This is useful
 * for typing all-caps identifiers like `MOD_MASK_ALT`.
 * Caps Word is activated by pressing the left and right shift keys at the same
 * time, or by tapping `CW_TOGG`.
 *
 * For full documentation, see
 * <https://getreuer.info/posts/keyboards/caps-word>
 */

#pragma once

#include "quantum.h"

#ifndef CAPS_WORD_ENABLE
#ifdef __cplusplus
extern "C" {
#endif

/** Turns on Caps Word. */
void caps_word_on(void);

/** Turns off Caps Word. */
void caps_word_off(void);

/** Toggles Caps Word on/off. */
void caps_word_toggle(void);

/** Returns true if Caps Word is currently active. */
bool is_caps_word_on(void);

/**
 * @brief Optional callback that gets called when Caps Word turns on or off.
 *
 * @param active True if Caps Word activates, false if it deactivates.
 */
void caps_word_set_user(bool active);

/**
 * @brief Optional callback called on each key press while Caps Word is active.
 *
 * Return true to continue Caps Word, or false to break the word and stop Caps Word.
 * Call `add_weak_mods(MOD_BIT(KC_LSFT))` to shift the key.
 *
 * @param keycode Keycode of the pressed key.
 * @return true to continue Caps Word, false to stop.
 */
bool caps_word_press_user(uint16_t keycode);

#ifdef __cplusplus
}
#endif
#endif  // CAPS_WORD_ENABLE
