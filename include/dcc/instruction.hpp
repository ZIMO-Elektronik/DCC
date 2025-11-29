// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Instruction
///
/// \file   dcc/instruction.hpp
/// \author Vincent Hamp
/// \date   04/01/2022

#pragma once

#include <cstdint>
#include <span>
#include "packet.hpp"

namespace dcc {

enum struct Instruction : uint8_t {
  UnknownService,     ///< Instruction is unknown or service
  DecoderControl,     ///< Instruction is decoder control
  ConsistControl,     ///< Instruction is consist control
  AdvancedOperations, ///< Instruction is advanced operations
  SpeedDirection,     ///< Instruction is speed and direction
  FunctionGroup,      ///< Instruction is function group
  FeatureExpansion,   ///< Instruction is feature expansion
  CvAccess,           ///< Instruction is CV access
  Logon               ///< Instruction is logon
};

/// Decode instruction
///
/// \tparam InputIt std::input_iterator
/// \param  first   Beginning of the range to decode from
/// \return Instruction
template<std::input_iterator InputIt>
requires(sizeof(std::iter_value_t<InputIt>) == 1uz)
constexpr Instruction decode_instruction(InputIt first) {
  switch (*first) {
    // Decoder control
    case 0b0000'0000u: [[fallthrough]];
    case 0b0000'0001u: [[fallthrough]];
    case 0b0000'0010u: [[fallthrough]];
    case 0b0000'0011u: [[fallthrough]];
    // case 0b0000'0100u:
    // case 0b0000'0101u:
    // case 0b0000'0110u:
    // case 0b0000'0111u:
    // case 0b0000'1000u:
    // case 0b0000'1001u:
    case 0b0000'1010u: [[fallthrough]];
    case 0b0000'1011u: [[fallthrough]];
    // case 0b0000'1100u:
    // case 0b0000'1101u:
    // case 0b0000'1110u:
    case 0b0000'1111u: return Instruction::DecoderControl;

    // Consist control
    // case 0b0001'0000u:
    // case 0b0001'0001u:
    case 0b0001'0010u: [[fallthrough]];
    case 0b0001'0011u: return Instruction::ConsistControl;
    // case 0b0001'0100u:
    // case 0b0001'0101u:
    // case 0b0001'0110u:
    // case 0b0001'0111u:
    // case 0b0001'1000u:
    // case 0b0001'1001u:
    // case 0b0001'1010u:
    // case 0b0001'1011u:
    // case 0b0001'1100u:
    // case 0b0001'1101u:
    // case 0b0001'1110u:
    // case 0b0001'1111u:

    // Advanced operations
    // case 0b0010'0000u:
    // case 0b0010'0001u:
    // case 0b0010'0010u:
    // case 0b0010'0011u:
    // case 0b0010'0100u:
    // case 0b0010'0101u:
    // case 0b0010'0110u:
    // case 0b0010'0111u:
    // case 0b0010'1000u:
    // case 0b0010'1001u:
    // case 0b0010'1010u:
    // case 0b0010'1011u:
    // case 0b0010'1100u:
    // case 0b0010'1101u:
    // case 0b0010'1110u:
    // case 0b0010'1111u:
    // case 0b0011'0000u:
    // case 0b0011'0001u:
    // case 0b0011'0010u:
    // case 0b0011'0011u:
    // case 0b0011'0100u:
    // case 0b0011'0101u:
    // case 0b0011'0110u:
    // case 0b0011'0111u:
    // case 0b0011'1000u:
    // case 0b0011'1001u:
    // case 0b0011'1010u:
    // case 0b0011'1011u:
    case 0b0011'1100u: [[fallthrough]];
    case 0b0011'1101u: [[fallthrough]];
    case 0b0011'1110u: [[fallthrough]];
    case 0b0011'1111u: return Instruction::AdvancedOperations;

    // Speed and direction
    case 0b0100'0000u: [[fallthrough]];
    case 0b0100'0001u: [[fallthrough]];
    case 0b0100'0010u: [[fallthrough]];
    case 0b0100'0011u: [[fallthrough]];
    case 0b0100'0100u: [[fallthrough]];
    case 0b0100'0101u: [[fallthrough]];
    case 0b0100'0110u: [[fallthrough]];
    case 0b0100'0111u: [[fallthrough]];
    case 0b0100'1000u: [[fallthrough]];
    case 0b0100'1001u: [[fallthrough]];
    case 0b0100'1010u: [[fallthrough]];
    case 0b0100'1011u: [[fallthrough]];
    case 0b0100'1100u: [[fallthrough]];
    case 0b0100'1101u: [[fallthrough]];
    case 0b0100'1110u: [[fallthrough]];
    case 0b0100'1111u: [[fallthrough]];
    case 0b0101'0000u: [[fallthrough]];
    case 0b0101'0001u: [[fallthrough]];
    case 0b0101'0010u: [[fallthrough]];
    case 0b0101'0011u: [[fallthrough]];
    case 0b0101'0100u: [[fallthrough]];
    case 0b0101'0101u: [[fallthrough]];
    case 0b0101'0110u: [[fallthrough]];
    case 0b0101'0111u: [[fallthrough]];
    case 0b0101'1000u: [[fallthrough]];
    case 0b0101'1001u: [[fallthrough]];
    case 0b0101'1010u: [[fallthrough]];
    case 0b0101'1011u: [[fallthrough]];
    case 0b0101'1100u: [[fallthrough]];
    case 0b0101'1101u: [[fallthrough]];
    case 0b0101'1110u: [[fallthrough]];
    case 0b0101'1111u: [[fallthrough]];
    case 0b0110'0000u: [[fallthrough]];
    case 0b0110'0001u: [[fallthrough]];
    case 0b0110'0010u: [[fallthrough]];
    case 0b0110'0011u: [[fallthrough]];
    case 0b0110'0100u: [[fallthrough]];
    case 0b0110'0101u: [[fallthrough]];
    case 0b0110'0110u: [[fallthrough]];
    case 0b0110'0111u: [[fallthrough]];
    case 0b0110'1000u: [[fallthrough]];
    case 0b0110'1001u: [[fallthrough]];
    case 0b0110'1010u: [[fallthrough]];
    case 0b0110'1011u: [[fallthrough]];
    case 0b0110'1100u: [[fallthrough]];
    case 0b0110'1101u: [[fallthrough]];
    case 0b0110'1110u: [[fallthrough]];
    case 0b0110'1111u: [[fallthrough]];
    case 0b0111'0000u: [[fallthrough]];
    case 0b0111'0001u: [[fallthrough]];
    case 0b0111'0010u: [[fallthrough]];
    case 0b0111'0011u: [[fallthrough]];
    case 0b0111'0100u: [[fallthrough]];
    case 0b0111'0101u: [[fallthrough]];
    case 0b0111'0110u: [[fallthrough]];
    case 0b0111'0111u: [[fallthrough]];
    case 0b0111'1000u: [[fallthrough]];
    case 0b0111'1001u: [[fallthrough]];
    case 0b0111'1010u: [[fallthrough]];
    case 0b0111'1011u: [[fallthrough]];
    case 0b0111'1100u: [[fallthrough]];
    case 0b0111'1101u: [[fallthrough]];
    case 0b0111'1110u: [[fallthrough]];
    case 0b0111'1111u: return Instruction::SpeedDirection;

    // Function group
    case 0b1000'0000u: [[fallthrough]];
    case 0b1000'0001u: [[fallthrough]];
    case 0b1000'0010u: [[fallthrough]];
    case 0b1000'0011u: [[fallthrough]];
    case 0b1000'0100u: [[fallthrough]];
    case 0b1000'0101u: [[fallthrough]];
    case 0b1000'0110u: [[fallthrough]];
    case 0b1000'0111u: [[fallthrough]];
    case 0b1000'1000u: [[fallthrough]];
    case 0b1000'1001u: [[fallthrough]];
    case 0b1000'1010u: [[fallthrough]];
    case 0b1000'1011u: [[fallthrough]];
    case 0b1000'1100u: [[fallthrough]];
    case 0b1000'1101u: [[fallthrough]];
    case 0b1000'1110u: [[fallthrough]];
    case 0b1000'1111u: [[fallthrough]];
    case 0b1001'0000u: [[fallthrough]];
    case 0b1001'0001u: [[fallthrough]];
    case 0b1001'0010u: [[fallthrough]];
    case 0b1001'0011u: [[fallthrough]];
    case 0b1001'0100u: [[fallthrough]];
    case 0b1001'0101u: [[fallthrough]];
    case 0b1001'0110u: [[fallthrough]];
    case 0b1001'0111u: [[fallthrough]];
    case 0b1001'1000u: [[fallthrough]];
    case 0b1001'1001u: [[fallthrough]];
    case 0b1001'1010u: [[fallthrough]];
    case 0b1001'1011u: [[fallthrough]];
    case 0b1001'1100u: [[fallthrough]];
    case 0b1001'1101u: [[fallthrough]];
    case 0b1001'1110u: [[fallthrough]];
    case 0b1001'1111u: [[fallthrough]];
    case 0b1010'0000u: [[fallthrough]];
    case 0b1010'0001u: [[fallthrough]];
    case 0b1010'0010u: [[fallthrough]];
    case 0b1010'0011u: [[fallthrough]];
    case 0b1010'0100u: [[fallthrough]];
    case 0b1010'0101u: [[fallthrough]];
    case 0b1010'0110u: [[fallthrough]];
    case 0b1010'0111u: [[fallthrough]];
    case 0b1010'1000u: [[fallthrough]];
    case 0b1010'1001u: [[fallthrough]];
    case 0b1010'1010u: [[fallthrough]];
    case 0b1010'1011u: [[fallthrough]];
    case 0b1010'1100u: [[fallthrough]];
    case 0b1010'1101u: [[fallthrough]];
    case 0b1010'1110u: [[fallthrough]];
    case 0b1010'1111u: [[fallthrough]];
    case 0b1011'0000u: [[fallthrough]];
    case 0b1011'0001u: [[fallthrough]];
    case 0b1011'0010u: [[fallthrough]];
    case 0b1011'0011u: [[fallthrough]];
    case 0b1011'0100u: [[fallthrough]];
    case 0b1011'0101u: [[fallthrough]];
    case 0b1011'0110u: [[fallthrough]];
    case 0b1011'0111u: [[fallthrough]];
    case 0b1011'1000u: [[fallthrough]];
    case 0b1011'1001u: [[fallthrough]];
    case 0b1011'1010u: [[fallthrough]];
    case 0b1011'1011u: [[fallthrough]];
    case 0b1011'1100u: [[fallthrough]];
    case 0b1011'1101u: [[fallthrough]];
    case 0b1011'1110u: [[fallthrough]];
    case 0b1011'1111u: return Instruction::FunctionGroup;

    // Feature expansion
    case 0b1100'0000u: [[fallthrough]];
    case 0b1100'0001u: [[fallthrough]];
    case 0b1100'0010u: [[fallthrough]];
    case 0b1100'0011u: [[fallthrough]];
    // case 0b1100'0100u:
    // case 0b1100'0101u:
    // case 0b1100'0110u:
    // case 0b1100'0111u:
    // case 0b1100'1000u:
    // case 0b1100'1001u:
    // case 0b1100'1010u:
    // case 0b1100'1011u:
    // case 0b1100'1100u:
    // case 0b1100'1101u:
    // case 0b1100'1110u:
    // case 0b1100'1111u:
    // case 0b1101'0000u:
    // case 0b1101'0001u:
    // case 0b1101'0010u:
    // case 0b1101'0011u:
    // case 0b1101'0100u:
    // case 0b1101'0101u:
    // case 0b1101'0110u:
    // case 0b1101'0111u:
    case 0b1101'1000u: [[fallthrough]];
    case 0b1101'1001u: [[fallthrough]];
    case 0b1101'1010u: [[fallthrough]];
    case 0b1101'1011u: [[fallthrough]];
    case 0b1101'1100u: [[fallthrough]];
    case 0b1101'1101u: [[fallthrough]];
    case 0b1101'1110u: [[fallthrough]];
    case 0b1101'1111u: return Instruction::FeatureExpansion;

    // CV access
    // case 0b1110'0000u:
    // case 0b1110'0001u:
    // case 0b1110'0010u:
    // case 0b1110'0011u:
    case 0b1110'0100u: [[fallthrough]];
    case 0b1110'0101u: [[fallthrough]];
    case 0b1110'0110u: [[fallthrough]];
    case 0b1110'0111u: [[fallthrough]];
    case 0b1110'1000u: [[fallthrough]];
    case 0b1110'1001u: [[fallthrough]];
    case 0b1110'1010u: [[fallthrough]];
    case 0b1110'1011u: [[fallthrough]];
    case 0b1110'1100u: [[fallthrough]];
    case 0b1110'1101u: [[fallthrough]];
    case 0b1110'1110u: [[fallthrough]];
    case 0b1110'1111u: [[fallthrough]];
    // case 0b1111'0000u:
    // case 0b1111'0001u:
    case 0b1111'0010u: [[fallthrough]];
    case 0b1111'0011u: [[fallthrough]];
    case 0b1111'0100u: [[fallthrough]];
    case 0b1111'0101u: [[fallthrough]];
    case 0b1111'0110u: [[fallthrough]];
    // case 0b1111'0111u:
    // case 0b1111'1000u:
    case 0b1111'1001u: return Instruction::CvAccess;
    // case 0b1111'1010u:
    // case 0b1111'1011u:
    // case 0b1111'1100u:
    // case 0b1111'1101u:

    // Logon
    case 0b1111'1110u: return Instruction::Logon;

    // CV access
    case 0b1111'1111u: [[fallthrough]];

    default: return Instruction::UnknownService;
  }
}

/// Decode instruction
///
/// \param  bytes Raw bytes
/// \return Instruction
constexpr Instruction decode_instruction(std::span<uint8_t const> bytes) {
  return decode_instruction(cbegin(bytes));
}

/// Decode instruction
///
/// \param  packet  Packet
/// \return Instruction
constexpr Instruction decode_instruction(Packet const& packet) {
  auto const offset{packet[0uz] >= 128u && packet[0uz] <= 252u ? 2 : 1};
  return decode_instruction(cbegin(packet) + offset);
}

} // namespace dcc
