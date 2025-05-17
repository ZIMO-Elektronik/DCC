// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Readable
///
/// \file   dcc/rx/readable.hpp
/// \author Vincent Hamp
/// \date   29/11/2022

#pragma once

#include <concepts>
#include <cstdint>

namespace dcc::rx {

template<typename T>
concept Readable =
  requires(T t, uint32_t cv_addr, uint8_t byte, bool bit, uint32_t pos) {
    { t.readCv(cv_addr) } -> std::convertible_to<uint8_t>;
    { t.readCv(cv_addr, byte) } -> std::convertible_to<uint8_t>;
    { t.readCv(cv_addr, bit, pos) } -> std::convertible_to<bool>;
  };

} // namespace dcc::rx
