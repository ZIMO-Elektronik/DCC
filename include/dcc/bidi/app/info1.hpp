// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi app:info1
///
/// \file   dcc/bidi/app/info1.hpp
/// \author Vincent Hamp
/// \date   21/12/2025

#pragma once

#include <cstdint>

namespace dcc::bidi::app {

struct Info1 {
  static constexpr uint8_t id{3u};
  enum Flags : uint8_t {
    TrackPolarity = 1u << 0u,
    EastWest = 1u << 1u,
    Driving = 1u << 2u,
    Consist = 1u << 3u,
    AddressingRequest = 1u << 4u
  } d{};
  constexpr bool operator==(Info1 const&) const = default;
};

} // namespace dcc::bidi::app
