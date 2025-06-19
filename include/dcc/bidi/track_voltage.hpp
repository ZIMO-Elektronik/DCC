// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi app:dyn track voltage
///
/// \file   dcc/bidi/track_voltage.hpp
/// \author Vincent Hamp
/// \date   15/06/2023

#pragma once

#include <algorithm>
#include "app/dyn.hpp"

namespace dcc::bidi {

struct TrackVoltage : app::Dyn {
  explicit constexpr TrackVoltage(int32_t mV)
    : app::Dyn{.d = static_cast<uint8_t>(std::max<int32_t>(0, mV - 5000) / 100),
               .x = 46u} {}
};

} // namespace dcc::bidi
