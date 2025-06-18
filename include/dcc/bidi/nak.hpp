// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi NAK
///
/// \file   dcc/bidi/nak.hpp
/// \author Vincent Hamp
/// \date   15/05/2025

#pragma once

#include <array>
#include <cstdint>

namespace dcc::bidi {

using Nak = uint8_t;

/// Instruction received correctly but not supported
inline constexpr uint8_t nak{0b0011'1100u};

} // namespace dcc::bidi
