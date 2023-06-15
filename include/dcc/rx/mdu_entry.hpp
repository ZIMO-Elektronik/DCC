// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// MDU entry
///
/// \file   dcc/rx/mdu_entry.hpp
/// \author Vincent Hamp
/// \date   29/11/2022

#pragma once

#include <concepts>
#include <cstdint>

namespace dcc::rx {

template<typename T>
concept MduEntry = requires(T t, uint32_t addr, uint8_t byte) {
                     { t.mduEntry(addr, byte) } -> std::same_as<void>;
                   };

}  // namespace dcc::rx