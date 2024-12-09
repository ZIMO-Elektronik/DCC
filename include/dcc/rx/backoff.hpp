// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Backoff logic
///
/// \file   dcc/rx/backoff.hpp
/// \author Vincent Hamp
/// \date   09/08/2023

#pragma once

#include <algorithm>
#include <climits>
#include <cstdint>
#include <cstdlib>

namespace dcc::rx {

/// Implements O(2^n) backoff logic
struct Backoff {
  constexpr operator bool() {
    if (_count) {
      --_count;
      return true;
    } else {
      _count = randomCount();
      _range = std::min<decltype(_range)>(_range + 1, 3);
      return false;
    }
  }

  /// Don't backoff next time
  void now() {
    _range = 0;
    _count = 0u;
  }

private:
  uint8_t randomCount() const {
    return static_cast<decltype(_count)>(rand() % (CHAR_BIT << _range));
  }

  int8_t _range{};
  uint8_t _count{};
};

} // namespace dcc::rx