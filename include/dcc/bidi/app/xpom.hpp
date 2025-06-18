// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi app:xpom
///
/// \file   dcc/bidi/app/xpom.hpp
/// \author Vincent Hamp
/// \date   17/06/2025

#pragma once

#include <cstdint>

namespace dcc::bidi::app {

struct Xpom {
  static constexpr std::array<uint8_t, 4uz> ids{8u, 9u, 10u, 11u};
  uint8_t ss{};
  constexpr bool operator==(Xpom const&) const = default;
};

} // namespace dcc::bidi::app
