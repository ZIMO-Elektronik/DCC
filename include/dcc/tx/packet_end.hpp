// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Packet end
///
/// \file   dcc/tx/packet_end.hpp
/// \author Vincent Hamp
/// \date   27/10/2026

#pragma once

#include <concepts>

namespace dcc::tx {

template<typename T>
concept PacketEnd = requires(T t) {
  { t.packetEnd() } -> std::same_as<void>;
};

} // namespace dcc::tx
