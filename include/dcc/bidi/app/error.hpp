// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi app:error
///
/// \file   dcc/bidi/app/error.hpp
/// \author Vincent Hamp
/// \date   19/12/2025

#pragma once

#include <cstdint>

namespace dcc::bidi::app {

struct Error {
  static constexpr uint8_t id{6u};
  constexpr bool operator==(Error const&) const = default;
};

} // namespace dcc::bidi::app
