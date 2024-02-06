// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Transmit timing
///
/// \file   dcc/tx/timing.hpp
/// \author Vincent Hamp
/// \date   06/02/2024

#pragma once

#include <cstdint>

namespace dcc::tx {

enum Timing {
  Bit1Min = 56u,         ///< Minimal timing for half a 1-bit
  Bit1 = 58u,            ///< Standard timing for half a 1-bit
  Bit1Max = 60u,         ///< Maximal timing for half a 1-bit
  Bit0Min = 97u,         ///< Minimal timing for half a 0-bit
  Bit0 = 100u,           ///< Standard timing for half a 0-bit
  Bit0Max = 114u,        ///< Maximal timing for half a 0-bit
  Bit0MaxAnalog = 9898u  ///< Maximal timing for half a 0-bit analog
};

}  // namespace dcc::tx