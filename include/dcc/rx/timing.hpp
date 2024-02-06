// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive timing
///
/// \file   dcc/rx/timing.hpp
/// \author Vincent Hamp
/// \date   06/02/2024

#pragma once

#include <cstdint>
#include "../bit.hpp"

namespace dcc::rx {

enum Timing {
  Bit1Min = 52u,          ///< Minimal timing for half a 1-bit
  Bit1 = 58u,             ///< Standard timing for half a 1-bit
  Bit1Max = 64u,          ///< Maximal timing for half a 1-bit
  Bit0Min = 90u,          ///< Minimal timing for half a 0-bit
  Bit0 = 100u,            ///< Standard timing for half a 0-bit
  Bit0Max = 119u,         ///< Maximal timing for half a 0-bit
  Bit0MaxAnalog = 10000u  ///< Maximal timing for half a 0-bit analog
};

/// Convert time to bit
///
/// \param  time  Time in µs
/// \return Bit
constexpr Bit time2bit(uint32_t time) {
  if (time >= Bit1Min && time <= Bit1Max) return _1;
  else if (time >= Bit0Min && time <= Bit0MaxAnalog) return _0;
  else return Invalid;
}

}  // namespace dcc::rx