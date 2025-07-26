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
 * @file speculative_hold.h
 * @brief Speculative Hold module
 *
 * With Speculative Hold, the hold function on mod-tap keys is activate
 * immediately on key down. Then depending on how the mod-tap key is settled,
 * the hold action is canceled (when the mod-tap is being tapped) or allowed to
 * continue (when the mod-tap is being held).
 *
 * "Speculative" holding behavior is useful to remove the input lag that
 * mod-tap keys normally have. An acute case is when using a mod-tap with an
 * external mouse to perform Ctrl + click, as this would normally require
 * waiting out the tapping term. With Speculative Hold, the mod is activated
 * immediately.
 *
 * By default, Speculative Hold applies to mod-tap keys where the mod is a Ctrl
 * or Shift mod. Define the get_speculative_hold() callback to customize this
 * behavior.
 *
 *
 * For full documentation, see
 * <https://getreuer.info/posts/keyboards/speculative-hold>
 */

#pragma once
#include "quantum.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Optional callback to define per-key where Speculative Hold applies.
 *
 * @param keycode    Keycode of the mod-tap key.
 * @param record     keyrecord_t for the event.
 * @return Whether to perform speculative holding for this key.
 */
bool get_speculative_hold(uint16_t keycode, keyrecord_t* record);

#ifdef __cplusplus
}
#endif
