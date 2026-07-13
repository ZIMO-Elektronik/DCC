// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi app:decoder_unique
///
/// \file   dcc/bidi/app/decoder_unique.hpp
/// \author Vincent Hamp
/// \date   02/07/2026

#pragma once

#include <array>
#include <cstdint>

namespace dcc::bidi::app {

struct DecoderUnique {
  static constexpr uint8_t id{15u};
  uint16_t mid{};
  std::array<uint8_t, 4uz> did{};
};

} // namespace dcc::bidi::app
