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
#include <functional>

using ::testing::_;
using ::testing::InSequence;

extern "C" {
#include "community_modules.h"
#include "achordion.h"
}

namespace {
bool process_record_user_default(uint16_t keycode, keyrecord_t* record) {
  return true;
}
std::function<bool(uint16_t, keyrecord_t*)> process_record_user_fun =
    process_record_user_default;
} // namespace

extern "C" bool process_record_user(uint16_t keycode, keyrecord_t* record) {
  return process_record_user_fun(keycode, record);
}

class AchordionTest : public TestFixture {
 public:
  void SetUp() override {
    process_record_user_fun = process_record_user_default;
  }
};

TEST_F(AchordionTest, opposite_hands_chord) {
  TestDriver driver;
  KeymapKey mod_tap_key_lhs(0, 1, 0, LALT_T(KC_P));
  KeymapKey left_regular(0, 2, 0, KC_B);
  KeymapKey right_regular(0, 7, 0, KC_A);
  KeymapKey mod_tap_key_rhs(0, 8, 0, RALT_T(KC_Q));
  set_keymap({mod_tap_key_lhs, left_regular, right_regular, mod_tap_key_rhs});

  // Mod-tap on LHS, regular on RHS.
  EXPECT_NO_REPORT(driver);
  mod_tap_key_lhs.press();  // Press mod-tap.
  idle_for(TAPPING_TERM + 1);  // Wait for QMK core to settle the mod-tap.
  VERIFY_AND_CLEAR(driver);

  // Press opposite-hand key -> resolves mod-tap key as HOLD (Alt), registers A.
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LALT));
    EXPECT_REPORT(driver, (KC_LALT, KC_A));
  }
  right_regular.press();  // Press opposite-hand key.
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release opposite-hand key -> unregisters A.
  EXPECT_REPORT(driver, (KC_LALT));
  right_regular.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release mod-tap key -> unregisters Alt.
  EXPECT_EMPTY_REPORT(driver);
  mod_tap_key_lhs.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // For symmetry, test mod-tap on RHS, regular on LHS.
  EXPECT_NO_REPORT(driver);
  mod_tap_key_rhs.press(); // Press RHS mod-tap key -> no report.
  idle_for(TAPPING_TERM + 1);
  VERIFY_AND_CLEAR(driver);

  // Press LHS regular key -> resolves RHS mod-tap as HOLD (Alt), registers B.
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_RALT));
    EXPECT_REPORT(driver, (KC_RALT, KC_B));
  }
  left_regular.press(); // Press LHS regular key.
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release LHS regular key -> unregisters B.
  EXPECT_REPORT(driver, (KC_RALT));
  left_regular.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release RHS mod-tap key -> unregisters Alt.
  EXPECT_EMPTY_REPORT(driver);
  mod_tap_key_rhs.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

TEST_F(AchordionTest, same_hand_chord) {
  TestDriver driver;
  KeymapKey mod_tap_key(0, 1, 0, LALT_T(KC_P));
  KeymapKey left_regular(0, 2, 0, KC_Q);
  set_keymap({mod_tap_key, left_regular});

  EXPECT_NO_REPORT(driver);
  mod_tap_key.press(); // Press mod-tap key -> no report.
  idle_for(TAPPING_TERM + 1);
  VERIFY_AND_CLEAR(driver);

  // Press same-hand key -> resolves mod-tap key as TAP (P), then Q.
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_P));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_Q));
  }
  left_regular.press(); // Press same-hand key -> resolves immediately.
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release same-hand key -> unregisters Q.
  EXPECT_EMPTY_REPORT(driver);
  left_regular.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release mod-tap key -> already resolved, no report.
  EXPECT_NO_REPORT(driver);
  mod_tap_key.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

TEST_F(AchordionTest, settled_by_timeout) {
  TestDriver driver;
  KeymapKey mod_tap_key(0, 1, 0, LALT_T(KC_P));
  set_keymap({mod_tap_key});

  // Press mod-tap key -> no report.
  EXPECT_NO_REPORT(driver);
  mod_tap_key.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Wait past achordion timeout (1000 ms by default) -> settles as HOLD (Alt).
  EXPECT_REPORT(driver, (KC_LALT));
  idle_for(1100);
  VERIFY_AND_CLEAR(driver);

  // Release mod-tap key.
  EXPECT_EMPTY_REPORT(driver);
  mod_tap_key.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

TEST_F(AchordionTest, combo_bypass) {
  TestDriver driver;
  KeymapKey mod_tap_key(0, 1, 0, LALT_T(KC_P));
  KeymapKey key_x(0, 7, 0, KC_X);
  KeymapKey key_y(0, 6, 0, KC_Y);
  set_keymap({mod_tap_key, key_x, key_y});

  EXPECT_NO_REPORT(driver);
  mod_tap_key.press();  // Press mod-tap key.
  idle_for(TAPPING_TERM + 1);
  VERIFY_AND_CLEAR(driver);

  // Tap combo (X and Y pressed together). Since it's a combo, it settles the
  // mod-tap key as HOLD (Alt). Then the combo triggers and outputs KC_ENTER.
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LALT));
    EXPECT_REPORT(driver, (KC_LALT, KC_ENT));
    EXPECT_REPORT(driver, (KC_LALT));
  }
  tap_combo({key_x, key_y});
  VERIFY_AND_CLEAR(driver);

  // Release mod-tap key -> unregisters Alt.
  EXPECT_EMPTY_REPORT(driver);
  mod_tap_key.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

TEST_F(AchordionTest, opposite_hands_layer_tap_chord) {
  TestDriver driver;
  KeymapKey layer_tap_key(0, 1, 0, LT(1, KC_P));
  KeymapKey right_regular_l0(0, 7, 0, KC_A);
  KeymapKey right_regular_l1(1, 7, 0, KC_B);
  set_keymap({layer_tap_key, right_regular_l0, right_regular_l1});

  // Press layer-tap key -> no report.
  EXPECT_NO_REPORT(driver);
  layer_tap_key.press();
  idle_for(TAPPING_TERM + 1);
  VERIFY_AND_CLEAR(driver);

  // Press opposite-hand key -> resolves LT as HOLD, registers B.
  EXPECT_REPORT(driver, (KC_B));
  right_regular_l0.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release opposite-hand key -> unregisters B.
  EXPECT_EMPTY_REPORT(driver);
  right_regular_l0.release();
  run_one_scan_loop();
  EXPECT_EQ(layer_state, 2); // Layer 1 is active (binary 2, i.e. 1 << 1).
  VERIFY_AND_CLEAR(driver);

  // Release layer-tap key -> Layer 1 deactivated.
  EXPECT_NO_REPORT(driver);
  layer_tap_key.release();
  run_one_scan_loop();
  EXPECT_EQ(layer_state, 0);
  VERIFY_AND_CLEAR(driver);
}

TEST_F(AchordionTest, same_hand_layer_tap_chord) {
  TestDriver driver;
  KeymapKey layer_tap_key(0, 1, 0, LT(1, KC_P));
  KeymapKey left_regular(0, 2, 0, KC_Q);
  set_keymap({layer_tap_key, left_regular});

  EXPECT_NO_REPORT(driver);
  layer_tap_key.press();  // Press layer-tap key.
  idle_for(TAPPING_TERM + 1);
  VERIFY_AND_CLEAR(driver);

  // Press same-hand key -> resolves layer-tap key as TAP (P), then Q.
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_P));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_Q));
  }
  left_regular.press();  // Press same-hand key -> resolves immediately.
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release same-hand key -> unregisters Q.
  EXPECT_EMPTY_REPORT(driver);
  left_regular.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release layer-tap key -> already resolved, no report.
  EXPECT_NO_REPORT(driver);
  layer_tap_key.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

TEST_F(AchordionTest, two_mod_taps_nested_press_opposite_hands) {
  TestDriver driver;
  KeymapKey mod_tap_key_lhs(0, 1, 0, LALT_T(KC_P));
  KeymapKey mod_tap_key_rhs(0, 8, 0, LALT_T(KC_Q));
  set_keymap({mod_tap_key_lhs, mod_tap_key_rhs});

  // Press LHS mod-tap key -> enqueued and blocked by Achordion.
  EXPECT_NO_REPORT(driver);
  mod_tap_key_lhs.press();
  idle_for(TAPPING_TERM + 2);
  // Press RHS mod-tap key -> resolves LHS mod-tap key as HOLD (Alt).
  mod_tap_key_rhs.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release RHS mod-tap key -> resolves as TAP (Q) under Alt.
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LALT));
    EXPECT_REPORT(driver, (KC_LALT, KC_Q));
    EXPECT_REPORT(driver, (KC_LALT));
  }
  mod_tap_key_rhs.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release LHS mod-tap key -> Alt released.
  EXPECT_EMPTY_REPORT(driver);
  mod_tap_key_lhs.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

TEST_F(AchordionTest, two_mod_taps_nested_press_same_hand) {
  TestDriver driver;
  KeymapKey mod_tap_key1(0, 1, 0, LALT_T(KC_P));
  KeymapKey mod_tap_key2(0, 2, 0, LALT_T(KC_Q));
  set_keymap({mod_tap_key1, mod_tap_key2});

  // Press mod-tap keys.
  EXPECT_NO_REPORT(driver);
  mod_tap_key1.press();
  idle_for(TAPPING_TERM + 1);
  mod_tap_key2.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release mod-tap key 2.
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_P));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_Q));
    EXPECT_EMPTY_REPORT(driver);
  }
  mod_tap_key2.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release mod-tap key 1 -> already resolved, no report.
  EXPECT_NO_REPORT(driver);
  mod_tap_key1.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

TEST_F(AchordionTest, two_mod_taps_one_regular_key) {
  TestDriver driver;
  KeymapKey mod_tap_key1(0, 1, 0, LALT_T(KC_A));
  KeymapKey mod_tap_key2(0, 8, 0, LGUI_T(KC_B));
  KeymapKey regular_key(0, 9, 0, KC_C);
  set_keymap({mod_tap_key1, mod_tap_key2, regular_key});

  // Press keys.
  EXPECT_NO_REPORT(driver);
  mod_tap_key1.press();
  idle_for(TAPPING_TERM + 1);
  mod_tap_key2.press();
  run_one_scan_loop();
  regular_key.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LALT));
    EXPECT_REPORT(driver, (KC_LALT, KC_B));
    EXPECT_REPORT(driver, (KC_LALT, KC_B, KC_C));
    EXPECT_REPORT(driver, (KC_LALT, KC_C));
  }
  mod_tap_key2.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release regular key -> unregisters C, Alt remains.
  EXPECT_REPORT(driver, (KC_LALT));
  regular_key.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release mod-tap key 1 -> unregisters Alt.
  EXPECT_EMPTY_REPORT(driver);
  mod_tap_key1.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

TEST_F(AchordionTest, roll_layer_tap_key_with_regular_key) {
  TestDriver driver;
  KeymapKey layer_tap_hold_key(0, 1, 0, LT(1, KC_P));
  KeymapKey regular_key(0, 2, 0, KC_A);
  KeymapKey layer_key(1, 2, 0, KC_B);
  set_keymap({layer_tap_hold_key, regular_key, layer_key});

  EXPECT_NO_REPORT(driver);
  layer_tap_hold_key.press(); // Press layer-tap-hold key.
  idle_for(TAPPING_TERM + 1);
  VERIFY_AND_CLEAR(driver);

  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_P));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_A));
  }
  regular_key.press(); // Press regular key.
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  EXPECT_NO_REPORT(driver);
  layer_tap_hold_key.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release regular key -> empty report.
  EXPECT_EMPTY_REPORT(driver);
  regular_key.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

TEST_F(AchordionTest, eager_modifiers) {
  TestDriver driver;
  KeymapKey mod_tap_key(0, 1, 0, LSFT_T(KC_P));
  KeymapKey left_regular(0, 2, 0, KC_A);
  set_keymap({mod_tap_key, left_regular});

  // Press eager mod-tap key -> enqueued by QMK, no report yet.
  EXPECT_NO_REPORT(driver);
  mod_tap_key.press();
  run_one_scan_loop();
  EXPECT_EQ(get_mods(), 0);
  VERIFY_AND_CLEAR(driver);

  // Once QMK settles, the event reaches Achordion and eager mod is applied.
  EXPECT_REPORT(driver, (KC_LSFT)); // Eager mod.
  idle_for(TAPPING_TERM + 1);
  EXPECT_EQ(get_mods(), MOD_BIT_LSHIFT);
  VERIFY_AND_CLEAR(driver);

  // Press same-hand key -> resolves mod-tap key as TAP (P), then registers A.
  // The eager mod is cancelled.
  {
    InSequence s;
    EXPECT_EMPTY_REPORT(driver); // Eager mod cancelled.
    EXPECT_REPORT(driver, (KC_P));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_A));
  }
  left_regular.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release same-hand key -> A released.
  EXPECT_EMPTY_REPORT(driver);
  left_regular.release();
  run_one_scan_loop();
  EXPECT_EQ(get_mods(), 0);
  VERIFY_AND_CLEAR(driver);

  // Release mod-tap key -> already resolved, no report.
  EXPECT_NO_REPORT(driver);
  mod_tap_key.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

TEST_F(AchordionTest, process_record_user_plumbing) {
  TestDriver driver;
  KeymapKey mod_tap_key(0, 1, 0, GUI_T(KC_P));
  set_keymap({mod_tap_key});

  int call_count = 0;
  uint16_t last_user_keycode = 0;
  bool last_user_pressed = false;

  process_record_user_fun = [&](uint16_t keycode, keyrecord_t* record) {
    ++call_count;
    last_user_keycode = keycode;
    last_user_pressed = record->event.pressed;
    return true;
  };

  EXPECT_NO_REPORT(driver);
  mod_tap_key.press(); // Press mod_tap_key.
  idle_for(TAPPING_TERM + 1);
  EXPECT_EQ(call_count, 0); // Achordion is still unsettled, not plumbed yet.

  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LGUI));
    EXPECT_EMPTY_REPORT(driver);
  }
  mod_tap_key.release();
  run_one_scan_loop();

  // Both press and release should have been plumbed.
  EXPECT_EQ(call_count, 2);
  EXPECT_EQ(last_user_keycode, GUI_T(KC_P));
  EXPECT_FALSE(last_user_pressed); // Last event was the release.
  VERIFY_AND_CLEAR(driver);
}

