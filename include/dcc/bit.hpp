// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Bit
///
/// \file   dcc/bit.hpp
/// \author Vincent Hamp
/// \date   29/11/2022

#pragma once

#include <cstdint>
#include "timing.hpp"

namespace dcc {

enum Bit : uint8_t { _0, _1, Invalid };

/// Convert time to bit
///
/// \param  time  Time in µs
/// \return Bit
constexpr Bit time2bit(uint32_t time) {
  if (time >= Bit1Min && time <= Bit1Max) return _1;
  else if (time >= Bit0Min && time <= Bit0MaxAnalog) return _0;
  else return Invalid;
}

}  // namespace dcc