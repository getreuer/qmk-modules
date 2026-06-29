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

#include "keyboard_report_util.hpp"
#include "keycode.h"
#include "test_common.hpp"
#include "test_fixture.hpp"
#include "test_keymap_key.hpp"

using ::testing::_;
using ::testing::InSequence;

extern "C" {
#include "community_modules.h"
#include "achordion.h"
}

class AchordionStreakTest : public TestFixture { };

// Test 1: Streak starts with a regular tap, continues through a mod-tap,
// resolving it as a tap even when chording opposite hand.
TEST_F(AchordionStreakTest, streak_resolves_opposite_hand_as_tap) {
  TestDriver driver;
  KeymapKey key_a_alt(0, 1, 0, ALT_T(KC_A));
  KeymapKey key_b(0, 2, 0, KC_B);
  KeymapKey key_k(0, 7, 0, KC_K);
  set_keymap({key_a_alt, key_b, key_k});

  // Tap B (starts streak).
  tap_key(key_b);
  idle_for(100); // 100 ms < 200 ms streak timeout.

  // Press A (ALT_T(KC_A)).
  EXPECT_NO_REPORT(driver);
  key_a_alt.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Press K (opposite hand).
  EXPECT_NO_REPORT(driver);
  key_k.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release keys -> SFT_T(KC_A) resolves as tap because streak is active.
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_A));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_K));
    EXPECT_EMPTY_REPORT(driver);
  }
  key_k.release();
  run_one_scan_loop();
  key_a_alt.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

// Test 2: Streak expires, mod-tap resolves as hold.
TEST_F(AchordionStreakTest, expired_streak_resolves_opposite_hand_as_hold) {
  TestDriver driver;
  KeymapKey key_a_sft(0, 1, 0, SFT_T(KC_A));
  KeymapKey key_b(0, 2, 0, KC_B);
  KeymapKey key_k(0, 7, 0, KC_K);
  set_keymap({key_a_sft, key_b, key_k});

  // Tap B (starts streak).
  tap_key(key_b);
  idle_for(205); // 205 ms > 200 ms streak timeout (expires)

  // Press A
  EXPECT_NO_REPORT(driver);
  key_a_sft.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Press K (opposite hand) -> buffered.
  EXPECT_NO_REPORT(driver);
  key_k.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release K (opposite hand) -> triggers Permissive Hold, resolving Shift.
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_REPORT(driver, (KC_LSFT, KC_K));
    EXPECT_REPORT(driver, (KC_LSFT));
  }
  key_k.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  EXPECT_EMPTY_REPORT(driver);
  key_a_sft.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

// Test 3: Tap-hold keys also continue the streak if they resolve as taps.
TEST_F(AchordionStreakTest, tap_hold_tap_continues_streak) {
  TestDriver driver;
  KeymapKey key_a_sft(0, 1, 0, SFT_T(KC_A));
  KeymapKey key_b(0, 2, 0, KC_B);
  KeymapKey key_l_sft(0, 8, 0, SFT_T(KC_L));
  KeymapKey key_k(0, 7, 0, KC_K);
  set_keymap({key_a_sft, key_b, key_l_sft, key_k});

  // Tap B (starts streak).
  tap_key(key_b);

  // Tap A before timeout -> settles as A tap, continuing the streak.
  EXPECT_REPORT(driver, (KC_A));
  EXPECT_EMPTY_REPORT(driver);
  tap_key(key_a_sft);
  VERIFY_AND_CLEAR(driver);

  // Press L (SFT_T(KC_L)) on RHS.
  EXPECT_NO_REPORT(driver);
  key_l_sft.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Press B on Left hand -> buffered.
  EXPECT_NO_REPORT(driver);
  key_b.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release keys -> streak is continued by A tap, L resolves as tap.
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_L));
    EXPECT_REPORT(driver, (KC_L, KC_B));
    EXPECT_REPORT(driver, (KC_B));
    EXPECT_EMPTY_REPORT(driver);
  }
  key_l_sft.release();
  run_one_scan_loop();
  key_b.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

// Test 4: Keys that do not continue the streak (e.g. Escape) end the streak.
TEST_F(AchordionStreakTest, non_streak_key_ends_streak) {
  TestDriver driver;
  KeymapKey key_a_alt(0, 1, 0, ALT_T(KC_A));
  KeymapKey key_b(0, 2, 0, KC_B);
  KeymapKey key_esc(0, 3, 0, KC_ESC);
  KeymapKey key_k(0, 7, 0, KC_K);
  set_keymap({key_a_alt, key_b, key_esc, key_k});

  // Tap B (starts streak)
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_B));
    EXPECT_EMPTY_REPORT(driver);
  }
  tap_key(key_b);

  // Tap Escape, ending the streak.
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_ESC));
    EXPECT_EMPTY_REPORT(driver);
  }
  tap_key(key_esc);

  // Press A (ALT_T(KC_A))
  EXPECT_NO_REPORT(driver);
  key_a_alt.press();
  idle_for(TAPPING_TERM + 1);
  VERIFY_AND_CLEAR(driver);

  // Press K (opposite hand).
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LALT));
    EXPECT_REPORT(driver, (KC_LALT, KC_K));
  }
  key_k.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release K (opposite hand).
  EXPECT_REPORT(driver, (KC_LALT));
  key_k.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  EXPECT_EMPTY_REPORT(driver);
  key_a_alt.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

