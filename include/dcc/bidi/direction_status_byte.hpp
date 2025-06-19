// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi app:dyn direction status byte
///
/// \file   dcc/bidi/direction_status_byte.hpp
/// \author Vincent Hamp
/// \date   15/06/2023

#pragma once

#include "app/dyn.hpp"

namespace dcc::bidi {

struct DirectionStatusByte : app::Dyn {
  explicit constexpr DirectionStatusByte(uint8_t byte)
    : app::Dyn{.d = byte, .x = 27u} {}
};

} // namespace dcc::bidi
