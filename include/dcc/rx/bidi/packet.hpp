// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive BiDi packet
///
/// \file   dcc/rx/bidi/packet.hpp
/// \author Vincent Hamp
/// \date   04/01/2022

#pragma once

#include <array>
#include <cstdint>

namespace dcc::rx::bidi {

using namespace ::dcc::bidi;

struct Packet {
  Channel2 data{};
  friend bool operator==(Packet const&, Packet const&) = default;
  uint8_t size{};
};

}  // namespace dcc::rx::bidi