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

namespace dcc {

enum class Instruction : uint8_t {
  UnknownService,      ///< Instruction is unknown or service
  DecoderControl,      ///< Instruction is decoder control
  ConsistControl,      ///< Instruction is consist control
  AdvancedOperations,  ///< Instruction is advanced operations
  SpeedDirection,      ///< Instruction is speed and direction
  FunctionGroup,       ///< Instruction is function group
  FeatureExpansion,    ///< Instruction is feature expansion
  CvLong,              ///< Instruction is long type CV access
  CvShort,             ///< Instruction is short type CV access
  Logon                ///< Instruction is logon
};

/// Decode instruction
///
/// \param  data  Pointer to instruction data
/// \return Instruction
constexpr Instruction decode_instruction(uint8_t const* data) {
  switch (*data & 0xF0u) {
    case 0b0000'0000u: return Instruction::DecoderControl;
    case 0b0001'0000u: return Instruction::ConsistControl;
    case 0b0010'0000u: [[fallthrough]];
    case 0b0011'0000u: return Instruction::AdvancedOperations;
    case 0b0100'0000u: [[fallthrough]];
    case 0b0101'0000u: [[fallthrough]];
    case 0b0110'0000u: [[fallthrough]];
    case 0b0111'0000u: return Instruction::SpeedDirection;
    case 0b1000'0000u: [[fallthrough]];
    case 0b1001'0000u: [[fallthrough]];
    case 0b1010'0000u: [[fallthrough]];
    case 0b1011'0000u: return Instruction::FunctionGroup;
    case 0b1100'0000u: [[fallthrough]];
    case 0b1101'0000u: return Instruction::FeatureExpansion;
    case 0b1110'0000u: return Instruction::CvLong;
    case 0b1111'0000u: return Instruction::CvShort;
    default: break;
  }
  if (*data == 0b1111'1110u) return Instruction::Logon;
  return Instruction::UnknownService;
}

}  // namespace dcc