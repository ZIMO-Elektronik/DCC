// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi app:dyn kmh
///
/// \file   dcc/bidi/kmh.hpp
/// \author Vincent Hamp
/// \date   15/06/2023

#pragma once

#include "app/dyn.hpp"

namespace dcc::bidi {

struct Kmh : app::Dyn {
  explicit constexpr Kmh(int32_t kmh)
    : Dyn{.d = static_cast<uint8_t>(kmh < 512 ? (kmh < 256 ? kmh : kmh - 256)
                                              : 255),
          .x = static_cast<uint8_t>(kmh < 256 ? 0u : 1u)} {}
};

} // namespace dcc::bidi
