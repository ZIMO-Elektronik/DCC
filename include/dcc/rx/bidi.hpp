// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi
///
/// \file   dcc/rx/bidi.hpp
/// \author Vincent Hamp
/// \date   03/07/2026

#pragma once

#include <concepts>
#include <cstdint>
#include <span>

namespace dcc::rx {

template<typename T>
concept BiDi = requires(T t, std::span<uint8_t const> bytes) {
  { t.transmitBiDi(bytes) } -> std::same_as<void>;
};

} // namespace dcc::rx
