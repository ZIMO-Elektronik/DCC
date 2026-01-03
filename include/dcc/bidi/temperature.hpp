// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi app:dyn temperature
///
/// \file   dcc/bidi/temperature.hpp
/// \author Vincent Hamp
/// \date   15/06/2023

#pragma once

#include <ztl/math.hpp>
#include "app/dyn.hpp"

namespace dcc::bidi {

struct Temperature : app::Dyn {
  consteval Temperature() : app::Dyn{.x = 26u} {}
  explicit constexpr Temperature(int32_t temp)
    : app::Dyn{
        .d = static_cast<uint8_t>(ztl::lerp<int32_t>(temp, -50, 205, 0, 255)),
        .x = 26u} {}
};

} // namespace dcc::bidi
