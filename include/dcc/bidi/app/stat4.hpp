// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi app:stat4
///
/// \file   dcc/bidi/app/stat4.hpp
/// \author Vincent Hamp
/// \date   19/12/2025

#pragma once

#include <cstdint>

namespace dcc::bidi::app {

struct Stat4 {
  static constexpr uint8_t id{3u};
  uint8_t d{};
  constexpr bool operator==(Stat4 const&) const = default;
};

} // namespace dcc::bidi::app
