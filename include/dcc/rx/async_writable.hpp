// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Asynchronously writable
///
/// \file   dcc/rx/async_writable.hpp
/// \author Vincent Hamp
/// \date   29/11/2022

#pragma once

#include <concepts>
#include <cstdint>

namespace dcc::rx {

template<typename T>
concept AsyncWritable =
  requires(T t, uint32_t cv_addr, uint8_t byte, bool bit, uint32_t pos) {
    {
      t.writeCv(cv_addr, byte, [](uint8_t) {})
    } -> std::same_as<void>;
    {
      t.writeCv(cv_addr, bit, pos, [](bool) {})
    } -> std::same_as<void>;
  };

} // namespace dcc::rx
