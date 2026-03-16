// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Transmit addresses
///
/// \file   dcc/tx/addresses.hpp
/// \author Vincent Hamp
/// \date   16/03/2026

#pragma once

#include "../address.hpp"

namespace dcc::tx {

struct Addresses {
  Address current{};
  Address last{};
};

} // namespace dcc::tx
