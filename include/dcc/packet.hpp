// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Packet
///
/// \file   dcc/packet.hpp
/// \author Vincent Hamp
/// \date   31/01/2022

#pragma once

#include <cstdint>
#include <ztl/inplace_vector.hpp>

namespace dcc {

using Packet = ztl::inplace_vector<uint8_t, DCC_MAX_PACKET_SIZE>;

} // namespace dcc
