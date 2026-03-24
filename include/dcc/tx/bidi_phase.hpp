// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi Phase
///
/// \file   dcc/tx/bidi_phase.hpp
/// \author Jonas Gahlert
/// \date   24/03/2026

#pragma once

#include <cstdint>
#include <utility>

namespace dcc::tx::detail {

/// BiDi Phase
enum class BiDiPhase : uint8_t {
  Pre = 0u,
  Start = 1u,
  Channel1 = 2u,
  Channel2 = 3u,
  End = 4u,
  Done = 5u,

  Upper = Done,
};

/// Pre-increment operator
constexpr BiDiPhase& operator++(BiDiPhase& e) {
  e = static_cast<BiDiPhase>(std::to_underlying(e) +
                             1u % std::to_underlying(BiDiPhase::Upper));
  return e;
}

/// Post-increment operator
constexpr auto operator++(BiDiPhase& e, int) {
  auto const r{e};
  ++e;
  return r;
}

} // namespace dcc::tx
