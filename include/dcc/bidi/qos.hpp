// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi app:dyn quality of service
///
/// \file   dcc/bidi/qos.hpp
/// \author Vincent Hamp
/// \date   29/10/2026

#pragma once

#include "app/dyn.hpp"

namespace dcc::bidi {

struct QoS : app::Dyn {
  consteval QoS() : app::Dyn{.x = 7u} {}
  explicit constexpr QoS(uint8_t qos) : app::Dyn{.d = qos, .x = 7u} {}
};

} // namespace dcc::bidi
