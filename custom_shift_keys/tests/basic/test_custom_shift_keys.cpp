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
}

class CustomShiftKeysTest : public TestFixture {};

TEST_F(CustomShiftKeysTest, unshifted_keys) {
  TestDriver driver;
  KeymapKey key_dot(0, 0, 0, KC_DOT);
  KeymapKey key_coln(0, 1, 0, KC_COLN);
  set_keymap({key_dot, key_coln});

  // Type an unshifted dot -> .
  EXPECT_REPORT(driver, (KC_DOT));
  key_dot.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  EXPECT_EMPTY_REPORT(driver);
  key_dot.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Type an unshifted coln -> : (which registers as Shift + semicolon)
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_REPORT(driver, (KC_LSFT, KC_SCLN));
  }
  key_coln.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_EMPTY_REPORT(driver);
  }
  key_coln.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

TEST_F(CustomShiftKeysTest, shifted_keys) {
  TestDriver driver;
  KeymapKey key_dot(0, 0, 0, KC_DOT);
  KeymapKey key_coln(0, 1, 0, KC_COLN);
  KeymapKey key_shift(0, 2, 0, KC_LSFT);
  set_keymap({key_dot, key_coln, key_shift});

  EXPECT_REPORT(driver, (KC_LSFT));
  key_shift.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Shift + dot should type ? (Shift + /)
  EXPECT_REPORT(driver, (KC_LSFT, KC_SLSH));
  key_dot.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  EXPECT_REPORT(driver, (KC_LSFT));
  key_dot.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Shift + colon should type ; (unshifted Semicolon)
  {
    InSequence s;
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_SCLN));
  }
  key_coln.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  EXPECT_REPORT(driver, (KC_LSFT));
  key_coln.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  EXPECT_EMPTY_REPORT(driver);
  key_shift.release(); // Release shift
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

TEST_F(CustomShiftKeysTest, one_shot_shifted_keys) {
  TestDriver driver;
  KeymapKey key_dot(0, 0, 0, KC_DOT);
  KeymapKey key_coln(0, 1, 0, KC_COLN);
  KeymapKey key_osm_shift(0, 2, 0, OSM(MOD_LSFT));
  set_keymap({key_dot, key_coln, key_osm_shift});

  // Test OSM shift + dot -> ?
  EXPECT_NO_REPORT(driver);
  key_osm_shift.press();
  run_one_scan_loop();
  key_osm_shift.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_REPORT(driver, (KC_LSFT, KC_SLSH));
  }
  key_dot.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_EMPTY_REPORT(driver);
  }
  key_dot.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Test OSM shift + colon -> ;
  EXPECT_NO_REPORT(driver);
  key_osm_shift.press();
  run_one_scan_loop();
  key_osm_shift.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  EXPECT_REPORT(driver, (KC_SCLN));
  key_coln.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  EXPECT_EMPTY_REPORT(driver);
  key_coln.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

TEST_F(CustomShiftKeysTest, multiple_held_keys) {
  TestDriver driver;
  KeymapKey key_dot(0, 0, 0, KC_DOT);
  KeymapKey key_coln(0, 1, 0, KC_COLN);
  KeymapKey key_shift(0, 2, 0, KC_LSFT);
  set_keymap({key_dot, key_coln, key_shift});

  // Hold Shift
  EXPECT_REPORT(driver, (KC_LSFT));
  key_shift.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Hold Dot -> ? (Shift + /)
  EXPECT_REPORT(driver, (KC_LSFT, KC_SLSH));
  key_dot.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // While still holding Dot, press Colon
  // -> release Dot's ?, then press Colon's ;
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_SCLN));
  }
  key_coln.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release Colon -> release ;
  EXPECT_REPORT(driver, (KC_LSFT));
  key_coln.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release Dot -> no report since Dot was already released above.
  EXPECT_NO_REPORT(driver);
  key_dot.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Press Dot again -> ? (Shift + /)
  EXPECT_REPORT(driver, (KC_LSFT, KC_SLSH));
  key_dot.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release Dot -> release ?, restore Shift
  EXPECT_REPORT(driver, (KC_LSFT));
  key_dot.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  // Release Shift
  EXPECT_EMPTY_REPORT(driver);
  key_shift.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

