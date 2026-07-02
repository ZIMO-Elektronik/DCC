// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi app:decoder_state
///
/// \file   dcc/bidi/app/decoder_state.hpp
/// \author Vincent Hamp
/// \date   02/07/2026

#pragma once

#include <cstdint>

namespace dcc::bidi::app {

struct DecoderState {
  static constexpr uint8_t id{13u};
};

} // namespace dcc::bidi::app
