// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi app:error
///
/// \file   dcc/bidi/app/error.hpp
/// \author Vincent Hamp
/// \date   19/12/2025

#pragma once

#include <cstdint>

namespace dcc::bidi::app {

struct Error {
  static constexpr uint8_t id{6u};
  enum Code : uint8_t {
    None = 0x00u,
    InvalidCommand = 0x01u,
    Overcurrent = 0x02u,
    Undervoltage = 0x03u,
    Fuse = 0x04u,
    Overtemperature = 0x05u,
    Feedback = 0x06u,
    ManualOperation = 0x07u,
    Signal = 0x10u,
    Servo = 0x20u,
    Internal = 0x3Fu,
    AdditionalErrors = 0x40u,
  } d{};
  constexpr bool operator==(Error const&) const = default;
};

} // namespace dcc::bidi::app
