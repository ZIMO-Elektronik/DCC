// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi bundled channels type alias
///
/// \file   dcc/bidi/bundled_channels.hpp
/// \author Vincent Hamp
/// \date   12/02/2023

#pragma once

#include "channel1.hpp"
#include "channel2.hpp"

namespace dcc::bidi {

inline constexpr auto bundled_channels_size{channel1_size + channel2_size};
using BundledChannels = std::array<uint8_t, bundled_channels_size>;

} // namespace dcc::bidi
