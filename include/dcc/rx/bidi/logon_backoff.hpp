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
    if (count_) {
      --count_;
      return true;
    } else {
      count_ = randomCount();
      range_ = std::min<decltype(range_)>(range_ + 1, 3);
      return false;
    }
  }

  /// Don't backoff next time
  void now() { count_ = 0u; }

  /// Reset
  void reset() {
    range_ = 0;
    count_ = randomCount();
  }

private:
  uint8_t randomCount() const {
    return static_cast<decltype(count_)>(rand() % (CHAR_BIT << range_));
  }

  int8_t range_{};
  uint8_t count_{randomCount()};
};

}  // namespace dcc::rx::bidi