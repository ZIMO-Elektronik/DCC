// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi app:get_data
///
/// \file   dcc/bidi/app/get_data.hpp
/// \author Vincent Hamp
/// \date   09/10/2026

#pragma once

#include <array>
#include <cstdint>

namespace dcc::bidi::app {

struct GetData {
  std::array<uint8_t, 6uz> d{};
  constexpr bool operator==(GetData const&) const = default;
};

} // namespace dcc::bidi::app
