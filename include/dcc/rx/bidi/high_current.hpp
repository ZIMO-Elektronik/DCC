// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// High current
///
/// \file   dcc/rx/bidi/high_current.hpp
/// \author Vincent Hamp
/// \date   29/11/2022

#pragma once

#include <concepts>

namespace dcc::rx::bidi {

template<typename T>
concept HighCurrent = requires(T t, bool high_current) {
  { t.highCurrentBiDi(high_current) } -> std::same_as<void>;
};

}  // namespace dcc::rx::bidi
