// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Track outputs
///
/// \file   dcc/tx/pre_cutout.hpp
/// \author Jonas Gahlert
/// \date   19/03/2025

#pragma once

#include <concepts>

namespace dcc::tx {

template<typename T>
concept PreCutout = requires(T t) {
  { t.preCutout() } -> std::same_as<void>;
};

} // namespace dcc::tx