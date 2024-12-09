// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Addresses
///
/// \file   dcc/addresses.hpp
/// \author Vincent Hamp
/// \date   04/01/2022

#pragma once

#include "address.hpp"

namespace dcc {

struct Addresses {
  Address primary{};
  Address consist{};
  Address logon{};
  Address received{};
};

} // namespace dcc