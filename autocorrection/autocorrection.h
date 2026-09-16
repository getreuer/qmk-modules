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
 * @file autocorrection.h
 * @brief Autocorrection community module: automatic typo detection and fixing.
 *
 * Overview
 * --------
 *
 * @note Autocorrect has evolved into core QMK feature Autocorrect! See
 * <https://docs.qmk.fm/features/autocorrect>
 *
 * Some words are more prone to typos than others. This module implements
 * rudimentary autocorrection, automatically detecting and fixing misspellings
 * using a compact trie stored in Flash memory.
 *
 * For full documentation, see
 * <https://getreuer.info/posts/keyboards/autocorrection>
 */

#pragma once

#include "quantum.h"

#ifndef AUTOCORRECT_ENABLE
#ifdef __cplusplus
extern "C" {
#endif

/** Returns true if autocorrection is currently enabled. */
bool is_autocorrection_on(void);

/** Turns on autocorrection. */
void autocorrection_on(void);

/** Turns off autocorrection. */
void autocorrection_off(void);

/** Toggles autocorrection on/off. */
void autocorrection_toggle(void);

#ifdef __cplusplus
}
#endif
#endif  // AUTOCORRECT_ENABLE
