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
#include <cmath>
#include <string>

using ::testing::NotNull;

#define TEST_HSV16(h, s, v) \
  ((((v) >> 4) << 12) | (((s) >> 4) << 8) | ((h) & 0xff))

extern "C" {
#include "quantum.h"
#include "palettefx.h"

// Mock the global led_config structure.
led_config_t g_led_config = {
    .matrix_co = {},
    .point = {},
    .flags = {}
};

// Mock driver callbacks.
static void mock_init(void) {}
static void mock_set_color(int index, uint8_t r, uint8_t g, uint8_t b) {}
static void mock_set_color_all(uint8_t r, uint8_t g, uint8_t b) {}
static void mock_flush(void) {}

// Mock the rgb_matrix_driver structure.
const rgb_matrix_driver_t rgb_matrix_driver = {
    .init = mock_init,
    .set_color = mock_set_color,
    .set_color_all = mock_set_color_all,
    .flush = mock_flush
};
}

// Floating-point representation of an HSV color for reference computations.
struct FloatHSV {
  float h;
  float s;
  float v;

  FloatHSV() : h(0.0f), s(0.0f), v(0.0f) {}
  FloatHSV(float hue, float sat, float val) : h(hue), s(sat), v(val) {}

  // Unpack from the 16-bit packed format: [v4:s4:h8].
  explicit FloatHSV(uint16_t hsv16)
      : h(static_cast<float>(hsv16 & 0xff)),
        s(static_cast<float>((hsv16 >> 8) & 0x0f) * 17.0f),
        v(static_cast<float>((hsv16 >> 12) & 0x0f) * 17.0f) {}
};

void PrintTo(const FloatHSV& color, ::std::ostream* os) {
  *os << "FloatHSV(" << static_cast<int>(std::round(color.h)) << ", "
                    << static_cast<int>(std::round(color.s)) << ", "
                    << static_cast<int>(std::round(color.v)) << ")";
}

void PrintTo(const hsv_t& color, ::std::ostream* os) {
  *os << "HSV("
    << (int)color.h << ", " << (int)color.s << ", " << (int)color.v
    << ")";
}

// Custom GMock matcher: checks that an hsv_t is approximately equal to a
// FloatHSV reference value, with circular wrap-around for hue.
MATCHER_P2(ApproxColor, expected, tol, "is approximately equal to color") {
  int hue_diff = std::abs(
      static_cast<int>(arg.h) - static_cast<int>(std::round(expected.h)));
  if (hue_diff > 128) {
    hue_diff = 256 - hue_diff;
  }
  int sat_diff = std::abs(
    static_cast<int>(arg.s) - static_cast<int>(std::round(expected.s)));
  int val_diff = std::abs(
    static_cast<int>(arg.v) - static_cast<int>(std::round(expected.v)));

  if (hue_diff > tol || sat_diff > tol || val_diff > tol) {
    *result_listener << "expected ≈ " << ::testing::PrintToString(expected);
    return false;
  }
  return true;
}

// Reference interpolation using floating-point math.
//
// This mirrors palettefx_interp_color() but uses float to independently verify
// the module's fixed-point lerp8by8/scale8 arithmetic. The implementation uses:
//   - scale8(i, s) = i * (1 + s) >> 8  (FASTLED_SCALE8_FIXED=1)
//   - lerp8by8(a, b, frac) = a +/- scale8(|b - a|, frac)
//   - frac = (x % 16) << 4  (a value in [0, 240])
//
// We replicate these integer operations in float to get the same rounding.
static FloatHSV float_interp_color(const uint16_t* palette, uint8_t x,
                                   uint8_t config_s, uint8_t config_v) {
  // Clamp x to [8, 247] and subtract 8, mapping to [0, 239].
  x = (x <= 8) ? 0 : ((x < 247) ? (x - 8) : 239);
  int i = x >> 4;
  float frac = (x % 16) / 16.0f;

  FloatHSV a(palette[i]);
  FloatHSV b(palette[i + 1]);

  // Linear interpolation with hue wrapping.
  float hue_diff = b.h - a.h;
  if (hue_diff > 128.0f) {
    hue_diff -= 256.0f;
  } else if (hue_diff < -128.0f) {
    hue_diff += 256.0f;
  }

  float result_h = a.h + frac * hue_diff;
  result_h = fmod(result_h, 256.0f);
  if (result_h < 0.0f) {
    result_h += 256.0f;
  }

  float result_s = a.s + frac * (b.s - a.s);
  result_s = (result_s * config_s) / 255.0f;

  float result_v = a.v + frac * (b.v - a.v);
  result_v = (result_v * config_v) / 255.0f;

  return {result_h, result_s, result_v};
}

class PaletteFxTest : public TestFixture { };

// Sweep all 256 input values and verify the interpolation output against the
// float reference model. Tests both full-scale and reduced matrix HSV configs.
TEST_F(PaletteFxTest, interpolation_full_sweep) {
  static const uint16_t test_palette[16] = {
    TEST_HSV16(139, 255,  85),
    TEST_HSV16(134, 255,  85),
    TEST_HSV16(131, 255, 102),
    TEST_HSV16(128, 255, 102),
    TEST_HSV16(127, 187, 102),
    TEST_HSV16(125, 119, 102),
    TEST_HSV16(124,  51, 102),
    TEST_HSV16(125,   0, 119),
    TEST_HSV16( 15,  17, 119), // Hue wrap: 125 → 15.
    TEST_HSV16( 17,  51, 153),
    TEST_HSV16( 18, 102, 170),
    TEST_HSV16( 19, 153, 204),
    TEST_HSV16( 21, 187, 238),
    TEST_HSV16( 23, 238, 255),
    TEST_HSV16( 26, 255, 255),
    TEST_HSV16( 30, 255, 255),
  };

  constexpr int kTolerance = 1;

  // Full saturation/value scaling.
  rgb_matrix_sethsv(rgb_matrix_get_hue(), 255, 255);

  for (int x = 0; x < 256; ++x) {
    SCOPED_TRACE("x=" + std::to_string(x));
    EXPECT_THAT(palettefx_interp_color(test_palette, x),
                ApproxColor(float_interp_color(test_palette, x, 255, 255),
                            kTolerance));
  }

  // Reduced scaling: 50% saturation, 75% value.
  rgb_matrix_sethsv(rgb_matrix_get_hue(), 128, 192);

  for (int x = 0; x < 256; ++x) {
    SCOPED_TRACE("x=" + std::to_string(x));
    EXPECT_THAT(palettefx_interp_color(test_palette, x),
                ApproxColor(float_interp_color(test_palette, x, 128, 192),
                            kTolerance));
  }
}

TEST_F(PaletteFxTest, palette_data_access) {
  const uint8_t num_palettes = palettefx_num_palettes();
  EXPECT_GT(num_palettes, 0);

  for (uint8_t hue : {0, 4, 8, 15, 32, 64, 125, 200, 240, 248, 255}) {
    SCOPED_TRACE("hue=" + std::to_string(hue));
    rgb_matrix_sethsv(hue, 255, 255);

    // Get current palette data (which may perform wrapped hue adjustments).
    const uint16_t* palette_data = palettefx_get_palette_data();
    EXPECT_THAT(palette_data, NotNull());

    uint8_t hue_after = rgb_matrix_get_hue();
    uint8_t expected_index = (hue_after / RGB_MATRIX_HUE_STEP) % num_palettes;
    const uint16_t* palette_data_by_idx =
      palettefx_get_palette_data_by_index(expected_index);
    EXPECT_THAT(palette_data_by_idx, NotNull());

    EXPECT_EQ(palette_data, palette_data_by_idx);
  }
}

