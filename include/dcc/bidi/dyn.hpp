// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi generic dyn
///
/// \file   dcc/bidi/dyn.hpp
/// \author Vincent Hamp
/// \date   15/06/2023

#pragma once

#include <cstdint>

namespace dcc::bidi {

/// Wrapper for dyn (ID7) data
struct Dyn {
  using value_type = int16_t;

  constexpr operator value_type&() { return d; }
  constexpr operator value_type const&() const { return d; }

  value_type d;  ///< DV (dynamic CV)
  uint8_t x;     ///< Subindex
};

}  // namespace dcc::bidi