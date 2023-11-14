// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Asynchronously readable
///
/// \file   dcc/rx/async_readable.hpp
/// \author Vincent Hamp
/// \date   29/11/2022

#pragma once

#include <concepts>
#include <cstdint>
#include <functional>

namespace dcc::rx {

template<typename T>
concept AsyncReadable = requires(T t,
                                 uint32_t cv_addr,
                                 uint8_t byte,
                                 bool bit,
                                 uint32_t pos,
                                 std::function<void(uint8_t)> byte_cb,
                                 std::function<void(bool)> bit_cb) {
  { t.readCv(cv_addr, byte, byte_cb) } -> std::same_as<void>;
  { t.readCv(cv_addr, bit, pos, bit_cb) } -> std::same_as<void>;
};

}  // namespace dcc::rx
