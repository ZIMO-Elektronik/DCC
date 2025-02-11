// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Track outputs
///
/// \file   dcc/tx/track_outputs.hpp
/// \author Vincent Hamp
/// \date   16/01/2024

#pragma once

#include <concepts>

namespace dcc::tx {

template<typename T>
concept TrackOutputs = requires(T t, bool N, bool P) {
  { t.trackOutputs(N, P) } -> std::same_as<void>;
};

} // namespace dcc::tx
