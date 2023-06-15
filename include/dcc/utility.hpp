// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Utility
///
/// \file   dcc/utility.hpp
/// \author Vincent Hamp
/// \date   29/11/2022

#pragma once

#include <cstdint>
#include <ztl/math.hpp>
#include "address.hpp"
#include "instruction.hpp"

namespace dcc {

/// Data to uint16_t
///
/// \param  data  Pointer to data
/// \return uint16_t from data
constexpr auto data2uint16(uint8_t const* data) {
  return static_cast<uint16_t>(data[0uz] << 8u | data[1uz] << 0u);
}

/// Data to uint32_t
///
/// \param  data  Pointer to data
/// \return uint32_t from data
constexpr auto data2uint32(uint8_t const* data) {
  return static_cast<uint32_t>(data[0uz] << 24u | data[1uz] << 16u |
                               data[2uz] << 8u | data[3uz] << 0u);
}

/// Scale notch from 14, 28 or 126 steps to 255
///
/// \tparam Scale Scaling
/// \param  notch Notch
/// \raturn Scaled notch
template<int32_t Scale>
constexpr int32_t scale_notch(int32_t notch)
  requires(Scale == 14 || Scale == 28 || Scale == 126)
{
  // Zero stays zero
  if (!notch) return 0;
  // One shares the same internal minimum across all scales
  constexpr int32_t max{255}, min{ztl::lerp<int32_t>(1, 0, 126, 0, max)};
  if (notch == 1) return min;
  if constexpr (Scale == 14 || Scale == 28)
    return ztl::lerp<int32_t>(notch, 1, Scale, min, max);
  else if constexpr (Scale == 126)
    return ztl::lerp<int32_t>(notch, 0, Scale, 0, max);
}

}  // namespace dcc