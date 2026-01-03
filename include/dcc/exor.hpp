// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Ex-or
///
/// \file   dcc/exor.hpp
/// \author Vincent Hamp
/// \date   13/02/2023

#pragma once

#include <cstdint>
#include <numeric>
#include <span>
#include "packet.hpp"

namespace dcc {

/// Exclusive disjunction (ex-or)
///
/// \param  bytes Bytes to calculate ex-or for
/// \return Ex-or
constexpr uint8_t exor(std::span<uint8_t const> bytes) {
  return std::accumulate(cbegin(bytes),
                         cend(bytes),
                         static_cast<uint8_t>(0u),
                         [](uint8_t a, uint8_t b) { return a ^ b; });
}

/// Exclusive disjunction (ex-or)
///
/// \param  packet  Packet
/// \return Ex-or
constexpr uint8_t exor(Packet const& packet) {
  // Packet checksum is not part of exor
  return exor({cbegin(packet), size(packet) - 1uz});
}

} // namespace dcc
