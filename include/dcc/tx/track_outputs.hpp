// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Track outputs
///
/// \file   dcc/tx/track_outputs.hpp
/// \author Vincent Hamp
/// \date   27/10/2026

#pragma once

#include <concepts>

namespace dcc::tx {

template<typename T>
concept TrackOutputs = requires(T& t, bool P, bool N) {
  { t.trackOutputs(P, N) } -> std::same_as<void>;
};

} // namespace dcc::tx
