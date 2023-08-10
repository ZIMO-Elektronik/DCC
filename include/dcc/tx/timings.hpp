// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Transmit timings
///
/// \file   dcc/tx/timings.hpp
/// \author Vincent Hamp
/// \date   31/01/2023

#pragma once

#include <algorithm>
#include <array>
#include <climits>
#include <cstdint>
#include <span>
#include <ztl/inplace_vector.hpp>
#include "config.hpp"

namespace dcc::tx {

using Timings =
  ztl::inplace_vector<uint16_t,
                      (std::numeric_limits<uint8_t>::max() +    // Preamble
                       1uz +                                    // Startbit
                       DCC_MAX_PACKET_SIZE * (CHAR_BIT + 1uz))  // Data
                        * 2uz>;                                 // Halfbit

/// Convert raw data into timings
///
/// \param  chunk Raw data
/// \param  cfg   Configuration
/// \return Timings
constexpr Timings raw2timings(std::span<uint8_t const> chunk, Config cfg = {}) {
  Timings timings{};
  auto first{begin(timings)};

  // Preamble
  auto const preamble_count{cfg.preamble_bits * 2uz};
  first =
    std::ranges::fill_n(first,
                        static_cast<Timings::difference_type>(preamble_count),
                        cfg.bit1_duration);

  // Data
  for (auto const byte : chunk) {
    // Startbit
    first = std::ranges::fill_n(first, 2uz, cfg.bit0_duration);
    for (auto i{CHAR_BIT}; i-- > 0;) {
      auto const bit{byte & (1u << i) ? cfg.bit1_duration : cfg.bit0_duration};
      first = std::ranges::fill_n(first, 2uz, bit);
    }
  }

  // Endbit
  first = std::ranges::fill_n(first, 2uz, cfg.bit1_duration);

  // Size
  timings.resize(static_cast<Timings::size_type>(
    std::ranges::distance(cbegin(timings), first)));

  return timings;
}

/// Convert packet into timings
///
/// \param  packet  Packet
/// \param  cfg     Configuration
/// \return Timings
constexpr Timings packet2timings(Packet const& packet, Config cfg = {}) {
  return raw2timings({cbegin(packet), size(packet)}, cfg);
}

}  // namespace dcc::tx