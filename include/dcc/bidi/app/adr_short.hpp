// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi app:adr_short
///
/// \file   dcc/bidi/app/adr_short.hpp
/// \author Vincent Hamp
/// \date   28/09/2026

#pragma once

#include <cstdint>

namespace dcc::bidi::app {

struct AdrShort {
  static constexpr uint8_t id{4u};
  uint8_t d{};
  constexpr bool operator==(AdrShort const&) const = default;
};

} // namespace dcc::bidi::app
