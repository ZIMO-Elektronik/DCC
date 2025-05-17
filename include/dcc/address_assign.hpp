// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Address assign
///
/// \file   dcc/address_assign.hpp
/// \author Vincent Hamp
/// \date   17/05/2025

#pragma once

#include <cstdint>

namespace dcc {

/// Address assign (RCN-218)
enum struct AddressAssign : uint8_t {
  Reserved = 0b00u | 0b01u,
  Permanent = 0b10u,
  Temporary = 0b11u,
};

} // namespace dcc
