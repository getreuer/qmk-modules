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

class SelectWordTest : public TestFixture {};

TEST_F(SelectWordTest, basic_select_word_tap) {
  TestDriver driver;
  KeymapKey key_selword(0, 0, 0, SELWORD);
  set_keymap({key_selword});

  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_RGHT));
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LEFT));
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LSFT));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LSFT, KC_RGHT));
    EXPECT_EMPTY_REPORT(driver);
  }

  tap_key(key_selword);
  VERIFY_AND_CLEAR(driver);

  // Tap the key again to extend the selection to the following word.
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LCTL, KC_LSFT));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LSFT, KC_RGHT));
  }

  key_selword.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  EXPECT_EMPTY_REPORT(driver);
  key_selword.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

TEST_F(SelectWordTest, basic_select_line_tap) {
  TestDriver driver;
  KeymapKey key_selline(0, 0, 0, SELLINE);
  set_keymap({key_selline});

  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_HOME));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_REPORT(driver, (KC_LSFT, KC_END));
    EXPECT_REPORT(driver, (KC_LSFT));
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
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_REPORT(driver, (KC_LSFT, KC_END)); // Select to end of the line.
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_EMPTY_REPORT(driver);
  }

  key_selline.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

TEST_F(SelectWordTest, basic_select_word_back_tap) {
  TestDriver driver;
  KeymapKey key_selwbak(0, 0, 0, SELWBAK);
  set_keymap({key_selwbak});

  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LEFT));
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_RGHT));
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LSFT));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LSFT, KC_LEFT));
    EXPECT_EMPTY_REPORT(driver);
  }

  tap_key(key_selwbak);
  VERIFY_AND_CLEAR(driver);

  // Tap the key again to extend the selection backward.
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LCTL, KC_LSFT));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LSFT, KC_LEFT));
  }

  key_selwbak.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  EXPECT_EMPTY_REPORT(driver);
  key_selwbak.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

TEST_F(SelectWordTest, basic_select_line_up_tap) {
  TestDriver driver;
  KeymapKey key_sellup(0, 0, 0, SELLUP);
  set_keymap({key_sellup});

  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_END));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_REPORT(driver, (KC_LSFT, KC_HOME));
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_EMPTY_REPORT(driver);
  }

  tap_key(key_sellup);
  VERIFY_AND_CLEAR(driver);

  // Tap the key again to extend the selection upward.
  {
    InSequence s;
    // Press SELLUP second time: Shift + Up
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_REPORT(driver, (KC_LSFT, KC_UP));
  }

  key_sellup.press();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);

  {
    InSequence s;
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_REPORT(driver, (KC_LSFT, KC_HOME));
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_EMPTY_REPORT(driver);
  }

  key_sellup.release();
  run_one_scan_loop();
  VERIFY_AND_CLEAR(driver);
}

TEST_F(SelectWordTest, select_word_timeout) {
  TestDriver driver;
  KeymapKey key_selword(0, 0, 0, SELWORD);
  set_keymap({key_selword});

  // Tap 1: Initial select word sequence
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_RGHT));
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LEFT));
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LSFT));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LSFT, KC_RGHT));
    EXPECT_EMPTY_REPORT(driver);
  }

  tap_key(key_selword);
  VERIFY_AND_CLEAR(driver);

  // Wait past SELECT_WORD_TIMEOUT to trigger reset.
  idle_for(SELECT_WORD_TIMEOUT + 100);

  // Tap 2: Since it timed out, it should trigger the INITIAL sequence again!
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_RGHT));
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LEFT));
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LSFT));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LSFT, KC_RGHT));
    EXPECT_EMPTY_REPORT(driver);
  }

  tap_key(key_selword);
  VERIFY_AND_CLEAR(driver);
}

TEST_F(SelectWordTest, select_word_interrupted) {
  TestDriver driver;
  KeymapKey key_selword(0, 0, 0, SELWORD);
  KeymapKey key_up(0, 1, 0, KC_UP);
  set_keymap({key_selword, key_up});

  // Tap 1: Initial select word sequence
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_RGHT));
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LEFT));
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LSFT));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LSFT, KC_RGHT));
    EXPECT_EMPTY_REPORT(driver);
  }

  tap_key(key_selword);
  VERIFY_AND_CLEAR(driver);

  // Tap KC_UP to interrupt selection
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_UP));
    EXPECT_EMPTY_REPORT(driver);
  }

  tap_key(key_up);
  VERIFY_AND_CLEAR(driver);

  // Tap 2: Since it was interrupted, it should trigger the INITIAL sequence
  // again!
  {
    InSequence s;
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_RGHT));
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LEFT));
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LSFT));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LSFT, KC_RGHT));
    EXPECT_EMPTY_REPORT(driver);
  }

  tap_key(key_selword);
  VERIFY_AND_CLEAR(driver);
}
