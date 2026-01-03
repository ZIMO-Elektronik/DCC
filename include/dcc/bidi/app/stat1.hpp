// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi app:stat1
///
/// \file   dcc/bidi/app/stat1.hpp
/// \author Vincent Hamp
/// \date   19/12/2025

#pragma once

#include <cstdint>

namespace dcc::bidi::app {

struct Stat1 {
  static constexpr uint8_t id{4u};
  uint8_t d{};
  constexpr bool operator==(Stat1 const&) const = default;
};

} // namespace dcc::bidi::app
