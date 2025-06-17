// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi ACKs
///
/// \file   dcc/bidi/acks.hpp
/// \author Vincent Hamp
/// \date   15/05/2025

#pragma once

#include <array>
#include <cstdint>

namespace dcc::bidi {

using Ack = uint8_t;

/// Instruction understood and will be executed
///
/// For some stupid, incomprehensible reason, there are two versions of ACK.
inline constexpr std::array<Ack, 2uz> acks{0b0000'1111u, 0b1111'0000u};

} // namespace dcc::bidi
