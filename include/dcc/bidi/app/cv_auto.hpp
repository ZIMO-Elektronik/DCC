// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi app:CV-auto
///
/// \file   dcc/bidi/app/cv_auto.hpp
/// \author Vincent Hamp
/// \date   17/06/2025

#pragma once

#include <cstdint>

namespace dcc::bidi::app {

struct CvAuto {
  static constexpr uint8_t id{12u};
  constexpr bool operator==(CvAuto const&) const = default;
};

} // namespace dcc::bidi::app
