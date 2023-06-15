// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi channel1 type alias
///
/// \file   dcc/bidi/channel1.hpp
/// \author Vincent Hamp
/// \date   12/02/2023

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace dcc::bidi {

inline constexpr auto channel1_size{2uz};
using Channel1 = std::array<uint8_t, channel1_size>;

}  // namespace dcc::bidi