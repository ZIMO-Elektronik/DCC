// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi timing
///
/// \file   dcc/bidi/timing.hpp
/// \author Vincent Hamp
/// \date   06/02/2024

#pragma once

#include <cstdint>

namespace dcc::bidi {

enum Timing {
  TCSMin = 26u,  ///< Minimal timing for cutout start
  TCS = 29u,     ///< Standard timing for cutout start
  TCSMax = 32u,  ///< Maximal timing for cutout start
  TCEMin = 454u, ///< Minimal timing for cutout end
  TCE = 471u,    ///< Standard timing for cutout end
  TCEMax = 488u, ///< Maximal timing for cutout end
  TTS1 = 80u,    ///< Minimal timing for start channel 1
  TTC1 = 177u,   ///< Maximal timing for end channel 1
  TTS2 = 193u,   ///< Minimal timing for start channel 2
  TTC2 = 454u    ///< Maximal timing for end channel 2
};

} // namespace dcc::bidi
