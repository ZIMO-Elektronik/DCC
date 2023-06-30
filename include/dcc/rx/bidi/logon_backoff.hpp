// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Logon backoff logic
///
/// \file   dcc/rx/bidi/logon_backoff.hpp
/// \author Vincent Hamp
/// \date   04/01/2022

#pragma once

#include <algorithm>
#include <climits>
#include <cstdint>
#include <cstdlib>

namespace dcc::rx::bidi {

/// Implements O(2^n) logon backoff logic
struct LogonBackoff {
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
  void now() { _count = 0u; }

  /// Reset
  void reset() {
    _range = 0;
    _count = randomCount();
  }

private:
  uint8_t randomCount() const {
    return static_cast<decltype(_count)>(rand() % (CHAR_BIT << _range));
  }

  int8_t _range{};
  uint8_t _count{randomCount()};
};

}  // namespace dcc::rx::bidi