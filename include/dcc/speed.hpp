// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Speed
///
/// \file   dcc/speed.hpp
/// \author Vincent Hamp
/// \date   06/06/2024

#pragma once

#include <cstdint>
#include <ztl/bits.hpp>
#include <ztl/math.hpp>

namespace dcc {

enum Speed : int32_t {
  EStop = -1,
  Stop = 0,
};

/// Scale speed from 14, 28 or 126 steps to 255
///
/// \tparam Scale Scaling
/// \param  speed Speed
/// \return Scaled speed
template<int32_t Scale>
constexpr int32_t scale_speed(int32_t speed)
  requires(Scale == 14 || Scale == 28 || Scale == 126)
{
  // EStop and Stop stay the same
  if (speed <= Stop) return speed;
  // One shares the same internal minimum across all scales
  constexpr int32_t max{255}, min{ztl::lerp<int32_t>(1, 0, 126, 0, max)};
  if (speed == 1) return min;
  if constexpr (Scale == 14 || Scale == 28)
    return ztl::lerp<int32_t>(speed, 1, Scale, min, max);
  else if constexpr (Scale == 126)
    return ztl::lerp<int32_t>(speed, 0, Scale, 0, max);
}

/// Decode direction and speed byte
///
/// \param  rggggg  Direction and speed byte
/// \retval -1      EStop
/// \retval 0       Stop
/// \retval >0      Speed
constexpr int32_t decode_rggggg(uint8_t rggggg, bool cv29_1 = true) {
  // Stop
  if (!(rggggg & 0b0000'1111u)) return Stop;
  // EStop
  else if (!(rggggg & 0b0000'1110u)) return EStop;

  // 14
  auto speed{static_cast<int32_t>(rggggg & 0b0000'1111u) - 1};

  // 28
  if (cv29_1) {
    speed <<= 1;
    if (speed && !(rggggg & ztl::mask<4u>)) --speed;
  }

  return speed;
}

/// Encode direction and speed byte
///
/// \param  dir   Direction
/// \param  speed Speed
/// \return Direction and speed byte
constexpr uint8_t encode_rggggg(bool dir, int32_t speed, bool cv29_1 = true) {
  // Halt
  if (!speed) return static_cast<uint8_t>(dir << 5u);
  // EStop
  else if (speed < 0) return static_cast<uint8_t>(dir << 5u) | 0b1u;

  auto rggggg{static_cast<uint32_t>(speed + 1)};

  // 28 speed steps with intermediate
  if (cv29_1) {
    rggggg = (rggggg >> 1) + 1;
    if (!(speed % 2)) rggggg |= ztl::mask<4u>;
  }

  return static_cast<uint8_t>(static_cast<uint32_t>(dir << 5u) | rggggg);
}

/// Decode direction and speed byte
///
/// \param  rggggggg  Direction and speed byte
/// \retval -1        EStop
/// \retval 0         Stop
/// \retval >0        Speed
constexpr int32_t decode_rggggggg(uint8_t rggggggg) {
  // Stop
  if (!(rggggggg & 0b0111'1111u)) return Stop;
  // EStop
  else if (!(rggggggg & 0b0111'1110u)) return EStop;
  // 126 speed steps
  else return (rggggggg & 0b0111'1111) - 1;
}

/// Encode direction and speed byte
///
/// \param  dir         Direction
/// \param  speed       Speed
/// \return Direction and speed byte
constexpr uint8_t encode_rggggggg(bool dir, int32_t speed) {
  // Halt
  if (!speed) return static_cast<uint8_t>(dir << 7u);
  // EStop
  else if (speed < 0) return static_cast<uint8_t>(dir << 7u) | 0b1u;
  // 126 speed steps
  else
    return static_cast<uint8_t>(static_cast<uint32_t>(dir << 7u) |
                                static_cast<uint32_t>(speed + 1));
}

} // namespace dcc
