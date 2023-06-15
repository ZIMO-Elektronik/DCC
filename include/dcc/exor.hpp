// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Ex-or
///
/// \file   dcc/exor.hpp
/// \author Vincent Hamp
/// \date   13/02/2023

#pragma once

#include <cstdint>
#include <numeric>
#include <span>

namespace dcc {

/// Exclusive disjunction (ex-or)
///
/// \param  chunk Chunk to calculate ex-or for
/// \return Ex-or
constexpr uint8_t exor(std::span<uint8_t const> chunk) {
  return std::accumulate(cbegin(chunk),
                         cend(chunk),
                         static_cast<uint8_t>(0u),
                         [](uint8_t a, uint8_t b) { return a ^ b; });
}

}  // namespace dcc