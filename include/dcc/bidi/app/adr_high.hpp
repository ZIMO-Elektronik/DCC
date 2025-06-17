// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi app:adr_high
///
/// \file   dcc/bidi/app/adr_high.hpp
/// \author Vincent Hamp
/// \date   17/06/2025

#pragma once

#include <cstdint>

namespace dcc::bidi::app {

struct AdrHigh {
  static constexpr uint8_t id{1u};
  uint8_t d{};
  constexpr bool operator==(AdrHigh const&) const = default;
};

} // namespace dcc::bidi::app
