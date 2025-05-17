// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// East-west
///
/// \file   dcc/rx/east_west.hpp
/// \author Vincent Hamp
/// \date   29/11/2022

#pragma once

#include <concepts>
#include <optional>

namespace dcc::rx {

template<typename T>
concept EastWest = requires(T t, uint16_t addr, std::optional<bool> opt_dir) {
  { t.eastWestDirection(addr, opt_dir) } -> std::same_as<void>;
};

} // namespace dcc::rx
