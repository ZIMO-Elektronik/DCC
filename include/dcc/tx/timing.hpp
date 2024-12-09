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
  Bit1Min = DCC_TX_MIN_BIT_1_TIMING, ///< Minimal timing for half a 1-bit
  Bit1 = 58u,                        ///< Standard timing for half a 1-bit
  Bit1Max = DCC_TX_MAX_BIT_1_TIMING, ///< Maximal timing for half a 1-bit
  Bit0Min = DCC_TX_MIN_BIT_0_TIMING, ///< Minimal timing for half a 0-bit
  Bit0 = 100u,                       ///< Standard timing for half a 0-bit
  Bit0Max = DCC_TX_MAX_BIT_0_TIMING, ///< Maximal timing for half a 0-bit
  Bit0MaxAnalog = 9898u              ///< Maximal timing for half a 0-bit analog
};

} // namespace dcc::tx