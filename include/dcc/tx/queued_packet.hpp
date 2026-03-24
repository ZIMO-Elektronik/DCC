// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Queued Packet
///
/// \file   dcc/tx/queued_packet.hpp
/// \author Jonas Gahlert
/// \date   24/03/2026

#pragma once

#include "../address.hpp"
#include "timings.hpp"
#include "timings_adapter.hpp"
#include <concepts>

namespace dcc::tx::detail {

/// QueuedPacket to preserve Address when using Timings
///
/// @tparam T Packet value type
template<typename T>
requires(std::same_as<T, Packet> || std::same_as<T, Timings>)
struct QueuedPacket {
    using value_type = std::conditional_t<std::same_as<T, Packet>, TimingsAdapter, Timings>;

    Address address{}; 
    value_type packet{};
};

}