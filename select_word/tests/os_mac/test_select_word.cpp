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

using ::testing::InSequence;

extern "C" {
#include "community_modules.h"
}

class SelectWordOsMacTest : public TestFixture {};

TEST_F(SelectWordOsMacTest, basic_select_word_tap) {
  TestDriver driver;
  KeymapKey key_selword(0, 0, 0, SELWORD);
  set_keymap({key_selword});

  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LALT));
    EXPECT_REPORT(driver, (KC_LALT, KC_RGHT));
    EXPECT_REPORT(driver, (KC_LALT));
    EXPECT_REPORT(driver, (KC_LALT, KC_LEFT));
    EXPECT_REPORT(driver, (KC_LALT));
    EXPECT_REPORT(driver, (KC_LALT, KC_LSFT));
    EXPECT_REPORT(driver, (KC_LALT, KC_LSFT, KC_RGHT));
    EXPECT_EMPTY_REPORT(driver);
  }

  tap_key(key_selword);
  VERIFY_AND_CLEAR(driver);

  // Tap the key again to extend the selection to the following word.
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LALT, KC_LSFT));
    EXPECT_REPORT(driver, (KC_LALT, KC_LSFT, KC_RGHT));
  }

  key_selword.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  EXPECT_EMPTY_REPORT(driver);
  key_selword.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

TEST_F(SelectWordOsMacTest, basic_select_line_tap) {
  TestDriver driver;
  KeymapKey key_selline(0, 0, 0, SELLINE);
  set_keymap({key_selline});

  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LGUI));
    EXPECT_REPORT(driver, (KC_LGUI, KC_LEFT));
    EXPECT_REPORT(driver, (KC_LGUI));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_LGUI, KC_LSFT));
    EXPECT_REPORT(driver, (KC_LGUI, KC_LSFT, KC_RGHT));
    EXPECT_REPORT(driver, (KC_LGUI, KC_LSFT));
    EXPECT_EMPTY_REPORT(driver);
  }

  tap_key(key_selline);
  VERIFY_AND_CLEAR(driver);

  // Tap the key again to extend the selection to the following line.
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_REPORT(driver, (KC_LSFT, KC_DOWN));
  }

  key_selline.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  {
    InSequence s;
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_LGUI));
    EXPECT_REPORT(driver, (KC_LGUI, KC_LSFT));
    EXPECT_REPORT(driver, (KC_LGUI, KC_LSFT, KC_RGHT));
    EXPECT_REPORT(driver, (KC_LGUI, KC_LSFT));
    EXPECT_REPORT(driver, (KC_LGUI));
    EXPECT_EMPTY_REPORT(driver);
  }
  key_selline.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}
