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
 * @file layer_lock.h
 * @brief Layer Lock community module: a key to stay in the current layer.
 *
 * Overview
 * --------
 *
 * @note Layer Lock has evolved into core QMK feature Layer Lock! See
 * <https://docs.qmk.fm/features/layer_lock>
 *
 * Layers are often accessed by holding a button, e.g. with a momentary layer
 * switch `MO(layer)` or layer tap `LT(layer, key)` key. But you may sometimes
 * want to "lock" or "toggle" the layer so that it stays on without having to
 * hold down a button. One way to do that is with a tap-toggle `TT` layer key,
 * but here is an alternative.
 *
 * This community module implements a "Layer Lock key" (`QK_LAYER_LOCK`, alias
 * `QK_LLCK`). When tapped, it "locks" the highest layer to stay active,
 * assuming the layer was activated by one of the following keys:
 *
 *  * `MO(layer)` momentary layer switch
 *  * `LT(layer, key)` layer tap
 *  * `OSL(layer)` one-shot layer
 *  * `TT(layer)` layer tap toggle
 *  * `LM(layer, mod)` layer-mod key (the layer is locked, but not the mods)
 *
 * Tapping the Layer Lock key again unlocks and turns off the layer.
 *
 * For full documentation, see
 * <https://getreuer.info/posts/keyboards/layer-lock>
 */

#pragma once

#include "quantum.h"

#ifndef LAYER_LOCK_ENABLE
#ifdef __cplusplus
extern "C" {
#endif

/** Returns true if `layer` is currently locked. */
bool is_layer_locked(uint8_t layer);

/** Locks and turns on `layer`. */
void layer_lock_on(uint8_t layer);

/** Unlocks and turns off `layer`. */
void layer_lock_off(uint8_t layer);

/** Unlocks and turns off all locked layers. */
void layer_lock_all_off(void);

/** Toggles whether `layer` is locked. */
void layer_lock_invert(uint8_t layer);

/**
 * @brief Optional callback called when a layer is locked or unlocked.
 *
 * @param locked_layers Bitfield where the kth bit represents whether the kth
 *                      layer is locked.
 */
void layer_lock_set_user(layer_state_t locked_layers);

#ifdef __cplusplus
}
#endif
#endif  // LAYER_LOCK_ENABLE
