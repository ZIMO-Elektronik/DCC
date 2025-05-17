// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Address group
///
/// \file   dcc/address_group.hpp
/// \author Vincent Hamp
/// \date   02/04/2024

#pragma once

#include <cstdint>

namespace dcc {

/// Address group (RCN-218)
enum struct AddressGroup : uint8_t {
  All = 0b00u,
  Loco = 0b01u,
  Acc = 0b10u,
  Now = 0b11u
};

} // namespace dcc
