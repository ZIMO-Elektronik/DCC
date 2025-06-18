// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi channel type aliases
///
/// \file   dcc/bidi/channel.hpp
/// \author Vincent Hamp
/// \date   12/02/2023

#pragma once

#include "datagram.hpp"

namespace dcc::bidi {

inline constexpr auto channel1_size{datagram_size<Bits::_12>};
using Channel1 = std::array<uint8_t, channel1_size>;

inline constexpr auto channel2_size{datagram_size<Bits::_36>};
using Channel2 = std::array<uint8_t, channel2_size>;

inline constexpr auto bundled_channels_size{channel1_size + channel2_size};
using BundledChannels = std::array<uint8_t, bundled_channels_size>;

} // namespace dcc::bidi
