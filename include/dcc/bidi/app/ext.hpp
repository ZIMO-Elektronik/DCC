// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi app:ext
///
/// \file   dcc/bidi/app/ext.hpp
/// \author Vincent Hamp
/// \date   17/06/2025

#pragma once

#include <cstdint>

namespace dcc::bidi::app {

struct Ext {
  static constexpr uint8_t id{3u};
  enum Type : uint8_t {
    AddressOnly = 0b0000u,
    Reserved8 = 0b1000u,
    Reserved9 = 0b1001u,
    GasStation = 0b1010u,
    CoalDepot = 0b1011u,
    WaterCrane = 0b1100u,
    SandStore = 0b1101u,
    ChargingStation = 0b1110u,
    FillingStation = 0b1111u
  } t{};
  uint16_t p{};
  constexpr bool operator==(Ext const&) const = default;
};

} // namespace dcc::bidi::app
