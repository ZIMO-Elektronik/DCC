// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Analog function
///
/// \file   dcc/rx/analog_function.hpp
/// \author Vincent Hamp
/// \date   30/10/2026

#pragma once

#include <concepts>
#include <cstdint>
#include "../address.hpp"

namespace dcc::rx {

template<typename T>
concept AnalogFunction =
  requires(T t, Address::value_type addr, uint8_t channel, uint8_t value) {
    { t.analogFunction(addr, channel, value) } -> std::same_as<void>;
  };

} // namespace dcc::rx
