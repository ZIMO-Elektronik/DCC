// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Direction
///
/// \file   dcc/direction.hpp
/// \author Vincent Hamp
/// \date   06/06/2024

#pragma once

namespace dcc {

enum Direction : bool {
  Forward = true,
  Backward = false,
  East = true,
  West = false,
};

} // namespace dcc