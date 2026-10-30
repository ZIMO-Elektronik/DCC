// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Binary state
///
/// \file   dcc/rx/binary_state.hpp
/// \author Vincent Hamp
/// \date   29/10/2026

#pragma once

#include <concepts>
#include <cstdint>
#include "../address.hpp"

namespace dcc::rx {

template<typename T>
concept BinaryState =
  requires(T t, Address::value_type addr, uint16_t bin_addr, bool state) {
    { t.binaryState(addr, bin_addr, state) } -> std::same_as<void>;
  };

} // namespace dcc::rx
