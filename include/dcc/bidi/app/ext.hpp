// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi app:ext
///
/// \file   dcc/bidi/app/ext.hpp
/// \author Vincent Hamp
/// \date   17/06/2025

#pragma once

#include <cstdint>

namespace dcc::bidi::app {

struct Ext {
  static constexpr uint8_t id{3u};
  constexpr bool operator==(Ext const&) const = default;
};

} // namespace dcc::bidi::app
