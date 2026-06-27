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
#include "mouse_report_util.hpp"
#include <cmath>
#include <utility>

extern "C" {
#include "community_modules.h"
#include "orbital_mouse.h"
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using ::testing::_;
using ::testing::InSequence;

static constexpr int kNumAngles = 64;
static constexpr int kNumSpeedCurveIntervals = 16;
static constexpr int kSpeedCurve[kNumSpeedCurveIntervals] =
    {24, 24, 24, 32, 58, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66};
static constexpr int kIntervalMs = 16;

// Matcher to check mouse report (x, y) coordinates with a tolerance.
MATCHER_P2(ApproxMouseReport, x, y,
    "approx mouse report (x: " + testing::PrintToString(x) +
    ", y: " + testing::PrintToString(y) + ")") {
  constexpr int tol = 1;
  if (std::abs(arg.x - x) > tol || std::abs(arg.y - y) > tol) {
    *result_listener << "mouse report (x: " << (int)arg.x
                     << ", y: " << (int)arg.y << ")";
    return false;
  }
  return true;
}

// Test reference of Orbital Mouse.
class ReferenceMouse {
 public:
  ReferenceMouse(): angle_step_(0) {}

  // Advance one interval and return the expected (x, y) mouse report delta.
  //   move_dir:  1 = forward (OM_U), -1 = backward (OM_D), 0 = no movement.
  //   steer_dir: 1 = counter-clockwise (OM_L), -1 = clockwise (OM_R), 0 = none.
  std::pair<int, int> next_interval(int move_dir, int steer_dir) {
    double dx = 0.0;
    double dy = 0.0;

    if (move_dir != 0) {
      if (move_t_ == 0) {
        speed_q9_6_ = kSpeedCurve[0] * 16;
      } else if (move_t_ <= 16 * (kNumSpeedCurveIntervals - 1)) {
        int i = (move_t_ - 1) / 16;
        speed_q9_6_ += kSpeedCurve[i + 1] - kSpeedCurve[i];
      }
      ++move_t_;
      int speed_q6_2 = (speed_q9_6_ + 8) / 16;

      // scaled_sin computes: (amplitude * lut[phase]) >> 2, yielding Q6.8.
      // Do the analogous ops with double trig.
      double angle_rad = (2.0 * M_PI * angle_step_) / kNumAngles;
      double speed_q6_8 = speed_q6_2 * 255.0 / 4.0;

      dx = -move_dir * speed_q6_8 * std::sin(angle_rad) / 256.0;
      dy = -move_dir * speed_q6_8 * std::cos(angle_rad) / 256.0;
    } else {
      move_t_ = 0;
      speed_q9_6_ = 0;
    }

    // Steering orbit displacement.
    if (steer_dir != 0) {
      double old_angle_rad = (2.0 * M_PI * angle_step_) / kNumAngles;
      angle_step_ = (angle_step_ + steer_dir) & (kNumAngles - 1);
      double new_angle_rad = (2.0 * M_PI * angle_step_) / kNumAngles;

      // The orbit center shifts when the angle changes. The cursor position
      // on the orbit circle moves, producing a displacement.
      double radius_pixels = kRadiusQ6_2 * 255.0 / (4.0 * 256.0);
      dx += radius_pixels * (std::sin(old_angle_rad) - std::sin(new_angle_rad));
      dy += radius_pixels * (std::cos(old_angle_rad) - std::cos(new_angle_rad));
    }

    x_accum_ += dx;
    y_accum_ += dy;
    int rx = static_cast<int>(std::trunc(x_accum_));
    x_accum_ -= rx;
    int ry = static_cast<int>(std::trunc(y_accum_));
    y_accum_ -= ry;

    return {rx, ry};
  }

 private:
  static constexpr int kRadiusQ6_2 = 36 * 4;

  double x_accum_ = 0.0;
  double y_accum_ = 0.0;
  uint8_t angle_step_;
  int move_t_ = 0;
  int speed_q9_6_ = 0;
};

class OrbitalMouseTest : public TestFixture {
 public:
  void TearDown() override {
    set_orbital_mouse_angle(0);
  }
};

// Test spiral movement: hold OM_U + OM_L to move forward while steering left.
// Verifies that the speed curve ramps up correctly and that combined
// movement + steering orbit displacement matches the reference model.
TEST_F(OrbitalMouseTest, spiral_movement_and_acceleration) {
  TestDriver driver;

  KeymapKey key_u(0, 1, 0, OM_U);
  KeymapKey key_l(0, 2, 0, OM_L);
  set_keymap({key_u, key_l});

  constexpr int kNumIntervals = 40;
  ReferenceMouse ref;

  {
    InSequence s;
    for (int t = 0; t < kNumIntervals; ++t) {
      auto [x, y] = ref.next_interval(/*move_dir=*/1, /*steer_dir=*/1);
      EXPECT_CALL(driver, send_mouse_mock(ApproxMouseReport(x, y)));
    }
  }

  key_u.press();
  key_l.press();
  run_one_scan_loop();

  idle_for(kNumIntervals * kIntervalMs);

  EXPECT_EMPTY_MOUSE_REPORT(driver);
  key_u.release();
  key_l.release();
  idle_for(kIntervalMs);
  VERIFY_AND_CLEAR(driver);
}

// Test clockwise rotation: hold OM_R for a full revolution (64 steps). Verifies
// that the angle wraps back to 0 and that the orbit displacement from steering
// alone produces the expected movements.
TEST_F(OrbitalMouseTest, spin_clockwise_circle) {
  TestDriver driver;

  KeymapKey key_r(0, 1, 0, OM_R);
  set_keymap({key_r});

  ReferenceMouse ref;

  {
    InSequence s;
    for (int t = 0; t < kNumAngles; ++t) {
      auto [x, y] = ref.next_interval(/*move_dir=*/0, /*steer_dir=*/-1);
      EXPECT_CALL(driver, send_mouse_mock(ApproxMouseReport(x, y)));
    }
  }

  key_r.press();
  idle_for(kNumAngles * kIntervalMs);

  EXPECT_EMPTY_MOUSE_REPORT(driver);
  key_r.release();
  idle_for(kIntervalMs);
  VERIFY_AND_CLEAR(driver);

  // After 64 CW steps, the angle should wrap back to 0.
  EXPECT_EQ(get_orbital_mouse_angle(), 0);
}

TEST_F(OrbitalMouseTest, cartesian_mode) {
  TestDriver driver;

  struct CartesianCase {
    uint16_t keycode;
    int angle;
    int expected_dx;
    int expected_dy;
  };

  const CartesianCase cases[] = {
    {OM_CS_U, 0,  0, -5},
    {OM_CS_L, 16, -5, 0},
    {OM_CS_D, 32, 0,  5},
    {OM_CS_R, 48, 5,  0},
  };

  for (const auto& tc : cases) {
    SCOPED_TRACE("angle=" + testing::PrintToString(tc.angle));
    if (timer_read() % 2 != 0) {
      idle_for(1);
    }
    KeymapKey key(0, 0, 0, tc.keycode);
    set_keymap({key});

    EXPECT_CALL(driver,
        send_mouse_mock(ApproxMouseReport(tc.expected_dx, tc.expected_dy)));
    key.press();
    run_one_scan_loop();
    idle_for(kIntervalMs);
    EXPECT_EQ(get_orbital_mouse_angle(), tc.angle);
    VERIFY_AND_CLEAR(driver);

    EXPECT_EMPTY_MOUSE_REPORT(driver);
    key.release();
    idle_for(kIntervalMs);
    VERIFY_AND_CLEAR(driver);
  }
}

