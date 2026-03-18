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
 * @file cyclotab.h
 * @brief Cyclotab community module
 *
 *
 * For full documentation, see
 * <https://getreuer.info/posts/keyboards/cyclotab>
 */

#pragma once

#include "quantum.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Returns the currently active hotkey, or KC_NO if inactive. */
uint16_t cyclotab_active_key(void);

/** Releases held mods and clears Cyclotab to initial state. */
void cyclotab_clear(void);

/** Optional user callback for setting the timeout. */ 
uint16_t cyclotab_timeout(uint16_t keycode);

#ifdef __cplusplus
}
#endif
