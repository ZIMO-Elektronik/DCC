// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Transmit configuration
///
/// \file   dcc/tx/config.hpp
/// \author Vincent Hamp
/// \date   12/06/2023

#pragma once

#include "timing.hpp"

namespace dcc::tx {

struct Config {
  /// Number of preamble bits [17-30]
  uint8_t num_preamble{DCC_TX_MIN_PREAMBLE_BITS};

  /// Duration of 1 bit [52-64]
  uint8_t bit1_duration{Bit1};

  /// Duration of 0 bit [90-119]
  uint8_t bit0_duration{Bit0};

  struct {
    /// Enable BiDi
    bool bidi{true};
  } flags{};
};

} // namespace dcc::tx
