// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Command station
///
/// \file   dcc/tx/command_station.hpp
/// \author Vincent Hamp
/// \date   15/02/2023

#pragma once

#include <concepts>

namespace dcc::tx {

template<typename T>
concept CommandStation = requires(T t, bool N, bool P) {
  { t.trackOutputs(N, P) } -> std::same_as<void>;
  { t.biDiStart() } -> std::same_as<void>;
  { t.biDiChannel1() } -> std::same_as<void>;
  { t.biDiChannel2() } -> std::same_as<void>;
  { t.biDiEnd() } -> std::same_as<void>;
};

}  // namespace dcc::tx