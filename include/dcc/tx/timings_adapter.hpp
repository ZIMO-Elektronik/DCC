// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Timings iterator/view which converts packet to timings
///
/// \file   dcc/tx/timings_adapter.hpp
/// \author Vincent Hamp
/// \date   28/05/2025

#pragma once

#include <cstdint>
#include <iterator>
#include <ranges>
#include <span>
#include "timings.hpp"

namespace dcc::tx {

/// TimingsAdapter
class TimingsAdapter : public std::ranges::view_interface<TimingsAdapter> {
  struct Sentinel {};

public:
  using value_type = Timings::value_type;
  using size_type = Timings::size_type;
  using difference_type = Timings::difference_type;
  using reference = value_type;
  using pointer = value_type;
  using iterator_category = std::input_iterator_tag;

  constexpr TimingsAdapter() = default;
  constexpr TimingsAdapter(Packet const& packet, Config cfg)
    : _packet{packet}, _cfg{cfg},
      _max_count{static_cast<size_type>(
        (_cfg.num_preamble + std::size(_packet) * (1uz + CHAR_BIT) + 1uz) *
        2uz)} {}
  constexpr TimingsAdapter(std::span<uint8_t const> bytes, Config cfg)
    : _cfg{cfg},
      _max_count{static_cast<size_type>(
        (_cfg.num_preamble + std::size(bytes) * (1uz + CHAR_BIT) + 1uz) *
        2uz)} {
    std::ranges::copy(bytes, std::back_inserter(_packet));
  }

  constexpr TimingsAdapter& operator++() {
    ++_count;
    return *this;
  }

  constexpr TimingsAdapter operator++(int) {
    auto retval{*this};
    ++(*this);
    return retval;
  }

  constexpr reference operator*() const {
    // Preamble
    auto const preamble_count{_cfg.num_preamble * 2uz};
    if (_count < preamble_count) return _cfg.bit1_duration;

    // Count without preamble
    auto i{_count - preamble_count};

    // Index of current byte
    auto const byte_index{
      static_cast<Packet::size_type>(i / ((1uz + CHAR_BIT) * 2uz))};
    if (byte_index >= std::size(_packet)) return _cfg.bit1_duration;

    // Index of current half bit
    auto const hbit_index{i % ((1uz + CHAR_BIT) * 2uz)};
    if (hbit_index < 2uz) return _cfg.bit0_duration;

    // Index of current bit
    auto const bit_index{(hbit_index - 2uz) / 2uz};
    return _packet[byte_index] & 1u << (CHAR_BIT - 1uz - bit_index)
             ? _cfg.bit1_duration
             : _cfg.bit0_duration;
  }

  constexpr bool operator==(Sentinel) const { return _count >= _max_count; }

  TimingsAdapter& begin() { return *this; }
  TimingsAdapter const& begin() const { return *this; }
  Sentinel end() { return {}; }
  Sentinel end() const { return {}; }
  Sentinel cend() { return {}; }
  Sentinel cend() const { return {}; }

private:
  Packet _packet;
  Config _cfg{};
  size_type _count{};
  size_type _max_count{};
};

constexpr auto begin(TimingsAdapter& c) -> decltype(c.begin()) {
  return c.begin();
}
constexpr auto begin(TimingsAdapter const& c) -> decltype(c.begin()) {
  return c.begin();
}
constexpr auto end(TimingsAdapter& c) -> decltype(c.end()) { return c.end(); }
constexpr auto end(TimingsAdapter const& c) -> decltype(c.end()) {
  return c.end();
}
constexpr auto cbegin(TimingsAdapter const& c) -> decltype(c.begin()) {
  return c.begin();
}
constexpr auto cend(TimingsAdapter const& c) -> decltype(c.end()) {
  return c.end();
}

} // namespace dcc::tx
