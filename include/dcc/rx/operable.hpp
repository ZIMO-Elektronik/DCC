// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Operable
///
/// \file   dcc/rx/operable.hpp
/// \author Vincent Hamp
/// \date   03/07/2026

#pragma once

#include <concepts>
#include <cstdint>
#include "../address.hpp"

namespace dcc::rx {

template<typename T>
concept Operable = requires(T t,
                            Address::value_type addr,
                            bool dir,
                            int32_t speed,
                            uint32_t mask,
                            uint32_t state) {
  { t.direction(addr, dir) } -> std::same_as<void>;
  { t.speed(addr, speed) } -> std::same_as<void>;
  { t.function(addr, mask, state) } -> std::same_as<void>;
};

} // namespace dcc::rx
