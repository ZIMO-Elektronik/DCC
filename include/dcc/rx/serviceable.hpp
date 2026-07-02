// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Serviceable
///
/// \file   dcc/rx/serviceable.hpp
/// \author Vincent Hamp
/// \date   03/07/2026

#pragma once

#include <concepts>

namespace dcc::rx {

template<typename T>
concept Serviceable = requires(T t, bool service_mode) {
  { t.serviceModeHook(service_mode) } -> std::same_as<void>;
  { t.serviceAck() } -> std::same_as<void>;
};

} // namespace dcc::rx
