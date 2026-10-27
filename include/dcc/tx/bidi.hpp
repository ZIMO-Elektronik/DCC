// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi
///
/// \file   dcc/tx/bidi.hpp
/// \author Vincent Hamp
/// \date   27/10/2026

#pragma once

#include <concepts>

namespace dcc::tx {

template<typename T>
concept BiDi = requires(T t) {
  { t.biDiStart() } -> std::same_as<void>;
  { t.biDiChannel1() } -> std::same_as<void>;
  { t.biDiChannel2() } -> std::same_as<void>;
  { t.biDiEnd() } -> std::same_as<void>;
};

} // namespace dcc::tx
