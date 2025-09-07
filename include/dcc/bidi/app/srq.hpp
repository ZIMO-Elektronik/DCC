// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi app:srq
///
/// \file   dcc/bidi/app/srq.hpp
/// \author Vincent Hamp
/// \date   03/09/2025

#pragma once

#include <cstdint>
#include "../../address.hpp"

namespace dcc::bidi::app {

struct Srq {
  Address::value_type d{};
  constexpr bool operator==(Srq const&) const = default;
};

} // namespace dcc::bidi::app
