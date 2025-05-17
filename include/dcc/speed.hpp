// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Speed
///
/// \file   dcc/speed.hpp
/// \author Vincent Hamp
/// \date   06/06/2024

#pragma once

#include <cstdint>

namespace dcc {

enum Speed : int32_t {
  EStop = -1,
  Stop = 0,
};

} // namespace dcc
