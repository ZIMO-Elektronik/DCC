// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Timing
///
/// \file   dcc/timing.hpp
/// \author Vincent Hamp
/// \date   04/01/2022

#pragma once

#include <cstdint>

namespace dcc {

enum Timing {
  Bit1Min = 52u,           ///< Minimal timing for half a 1-bit
  Bit1Norm = 58u,          ///< Norm timing for half a 1-bit
  Bit1Max = 64u,           ///< Maximal timing for half a 1-bit
  Bit0Min = 90u,           ///< Minimal timing for half a 0-bit
  Bit0Norm = 100u,         ///< Norm timing for half a 0-bit
  Bit0Max = 119u,          ///< Maximal timing for half a 0-bit
  Bit0MaxAnalog = 10000u,  ///< Maximal timing for half a 0-bit analog
  BiDiTCSMin = 26u,        ///< Minimal timing for BiDi cutout start
  BiDiTCS = 29u,           ///< Norm timing for BiDi cutout start
  BiDiTCSMax = 32u,        ///< Maximal timing for BiDi cutout start
  BiDiTCEMin = 454u,       ///< Minimal timing for BiDi cutout end
  BiDiTCE = 471u,          ///< Norm timing for BiDi cutout end
  BiDiTCEMax = 488u,       ///< Maximal timing for BiDi cutout end
  BiDiTTS1 = 80u,          ///< Minimal timing for BiDi start channel 1
  BiDiTTC1 = 177u,         ///< Maximal timing for BiDi end channel 1
  BiDiTTS2 = 193u,         ///< Minimal timing for BiDi start channel 2
  BiDiTTC2 = 454u,         ///< Maximal timing for BiDi end channel 2
};

}  // namespace dcc