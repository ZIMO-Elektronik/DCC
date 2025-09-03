// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Utility
///
/// \file   dcc/utility.hpp
/// \author Vincent Hamp
/// \date   29/11/2022

#pragma once

#include <cassert>
#include <concepts>
#include <cstdint>
#include <utility>
#include <ztl/math.hpp>
#include "address.hpp"
#include "address_assign.hpp"
#include "address_group.hpp"
#include "crc8.hpp"
#include "exor.hpp"
#include "instruction.hpp"
#include "packet.hpp"

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

/// uint16_t to data
///
/// \param  hword Half-word to convert
/// \param  data  Pointer to write to
/// \return Pointer after last element
constexpr auto uint16_2data(uint16_t hword, uint8_t* data) {
  *data++ = static_cast<uint8_t>((hword & 0xFF00u) >> 8u);
  *data++ = static_cast<uint8_t>((hword & 0x00FFu) >> 0u);
  return data;
}

/// uint32_t to data
///
/// \param  word  Word to convert
/// \param  data  Pointer to write to
/// \return Pointer after last element
constexpr auto uint32_2data(uint32_t word, uint8_t* data) {
  *data++ = static_cast<uint8_t>((word & 0xFF00'0000u) >> 24u);
  *data++ = static_cast<uint8_t>((word & 0x00FF'0000u) >> 16u);
  *data++ = static_cast<uint8_t>((word & 0x0000'FF00u) >> 8u);
  *data++ = static_cast<uint8_t>((word & 0x0000'00FFu) >> 0u);
  return data;
}

/// Scale speed from 14, 28 or 126 steps to 255
///
/// \tparam Scale Scaling
/// \param  speed Speed
/// \return Scaled speed
template<int32_t Scale>
constexpr int32_t scale_speed(int32_t speed)
  requires(Scale == 14 || Scale == 28 || Scale == 126)
{
  // EStop and Stop stay the same
  if (speed <= Stop) return speed;
  // One shares the same internal minimum across all scales
  constexpr int32_t max{255}, min{ztl::lerp<int32_t>(1, 0, 126, 0, max)};
  if (speed == 1) return min;
  if constexpr (Scale == 14 || Scale == 28)
    return ztl::lerp<int32_t>(speed, 1, Scale, min, max);
  else if constexpr (Scale == 126)
    return ztl::lerp<int32_t>(speed, 0, Scale, 0, max);
}

/// Make idle packet
///
/// \return Idle packet
consteval auto make_idle_packet() { return Packet{0xFFu, 0x00u, 0xFFu}; }

/// Make reset packet
///
/// \return Reset packet
consteval auto make_reset_packet() { return Packet{0x00u, 0x00u, 0x00u}; }

/// Make consist control packet
///
/// \param  addr  Address
/// \param  cv19  CV19
/// \return Consist control packet
constexpr auto make_consist_control_packet(Address addr, uint8_t cv19) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = static_cast<uint8_t>(0b0001'0010u | (cv19 >> 7u));
  *last++ = static_cast<uint8_t>(cv19 & 0b0111'1111u);
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make consist control packet
///
/// \param  addr  Address
/// \param  cv19  CV19
/// \return Consist control packet
constexpr auto make_consist_control_packet(Address::value_type addr,
                                           uint8_t cv19) {
  return make_consist_control_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco}, cv19);
}

/// Make advanced operations speed, direction and functions
///
/// \param  addr      Address
/// \param  rggggggg  Direction and speed byte
/// \param  f7_0      F7-0
/// \param  fs...     Functions
/// \return Advanced operations speed, direction and functions packet
constexpr auto make_advanced_operations_speed_direction_and_functions_packet(
  Address addr,
  uint8_t rggggggg,
  uint8_t f7_f0,
  std::unsigned_integral auto... fs) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b0011'1100u;
  *last++ = rggggggg;
  *last++ = f7_f0;
  ((*last++ = static_cast<uint8_t>(fs)), ...);
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make advanced operations speed, direction and functions
///
/// \param  addr      Address
/// \param  rggggggg  Direction and speed byte
/// \param  f7_0      F7-0
/// \param  fs...     Functions
/// \return Advanced operations speed, direction and functions packet
constexpr auto make_advanced_operations_speed_direction_and_functions_packet(
  Address::value_type addr,
  uint8_t rggggggg,
  uint8_t f7_f0,
  std::unsigned_integral auto... fs) {
  return make_advanced_operations_speed_direction_and_functions_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco},
    rggggggg,
    f7_f0,
    fs...);
}

/// Make advanced operations speed packet
///
/// \param  addr      Address
/// \param  rggggggg  Direction and speed byte
/// \return Advanced operations speed packet
constexpr auto make_advanced_operations_speed_packet(Address addr,
                                                     uint8_t rggggggg) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b0011'1111u;
  *last++ = rggggggg;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make advanced operations speed packet
///
/// \param  addr      Address
/// \param  rggggggg  Direction and speed byte
/// \return Advanced operations speed packet
constexpr auto make_advanced_operations_speed_packet(Address::value_type addr,
                                                     uint8_t rggggggg) {
  return make_advanced_operations_speed_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco},
    rggggggg);
}

/// \todo
constexpr auto make_advanced_operations_restricted_speed_packet(Address addr) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  return packet;
}

/// \todo
constexpr auto
make_advanced_operations_restricted_speed_packet(Address::value_type) {
  Packet packet{};
  return packet;
}

/// \todo
constexpr auto
make_advanced_operations_analog_function_group_packet(Address addr) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  return packet;
}

/// \todo
constexpr auto
make_advanced_operations_analog_function_group_packet(Address::value_type) {
  Packet packet{};
  return packet;
}

/// Make speed and direction packet
///
/// \param  addr    Address
/// \param  rggggg  Direction and speed byte
/// \return Speed and direction packet
constexpr auto make_speed_and_direction_packet(Address addr, uint8_t rggggg) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b0100'0000u | rggggg;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make speed and direction packet
///
/// \param  addr    Address
/// \param  rggggg  Direction and speed byte
/// \return Speed and direction packet
constexpr auto make_speed_and_direction_packet(Address::value_type addr,
                                               uint8_t rggggg) {
  return make_speed_and_direction_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco}, rggggg);
}

/// Make function group packet for F4-F0
///
/// \param  addr  Address
/// \param  state F4-F0 state
/// \return Function group packet for F4-0
constexpr auto make_function_group_f4_f0_packet(Address addr, uint8_t state) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = static_cast<uint8_t>(0b1000'0000u | (state & 0b1u) << 4u |
                                 (state & 0x1Fu) >> 1u);
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make function group packet for F4-F0
///
/// \param  addr  Address
/// \param  state F4-F0 state
/// \return Function group packet for F4-0
constexpr auto make_function_group_f4_f0_packet(Address::value_type addr,
                                                uint8_t state) {
  return make_function_group_f4_f0_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco}, state);
}

/// Make function group packet for F8-F5
///
/// \param  addr  Address
/// \param  state F8-F5 state
/// \return Function group packet for F8-F5
constexpr auto make_function_group_f8_f5_packet(Address addr, uint8_t state) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b1011'0000u | (state & 0xFu);
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make function group packet for F8-F5
///
/// \param  addr  Address
/// \param  state F8-F5 state
/// \return Function group packet for F8-F5
constexpr auto make_function_group_f8_f5_packet(Address::value_type addr,
                                                uint8_t state) {
  return make_function_group_f8_f5_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco}, state);
}

/// Make function group packet for F12-F9
///
/// \param  addr    Address
/// \param  f12_f9  F12-F9 state
/// \return Function group packet for F12-F9
constexpr auto make_function_group_f12_f9_packet(Address addr, uint8_t f12_f9) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b1010'0000u | (f12_f9 & 0xFu);
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make function group packet for F12-F9
///
/// \param  addr    Address
/// \param  f12_f9  F12-F9 state
/// \return Function group packet for F12-F9
constexpr auto make_function_group_f12_f9_packet(Address::value_type addr,
                                                 uint8_t f12_f9) {
  return make_function_group_f12_f9_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco}, f12_f9);
}

/// Make feature expansion packet for F20-F13
///
/// \param  addr    Address
/// \param  f20_f13 F20-F13 state
/// \return Feature expansion packet for F20-F13
constexpr auto make_feature_expansion_f20_f13_packet(Address addr,
                                                     uint8_t f20_f13) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b1101'1110u;
  *last++ = f20_f13;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make feature expansion packet for F20-F13
///
/// \param  addr    Address
/// \param  f20_f13 F20-F13 state
/// \return Feature expansion packet for F20-F13
constexpr auto make_feature_expansion_f20_f13_packet(Address::value_type addr,
                                                     uint8_t f20_f13) {
  return make_feature_expansion_f20_f13_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco}, f20_f13);
}

/// Make feature expansion packet for F28-F21
///
/// \param  addr    Address
/// \param  f28_f21 F28-F21 state
/// \return Feature expansion packet for F28-F21
constexpr auto make_feature_expansion_f28_f21_packet(Address addr,
                                                     uint8_t f28_f21) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b1101'1111u;
  *last++ = f28_f21;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make feature expansion packet for F28-F21
///
/// \param  addr    Address
/// \param  f28_f21 F28-F21 state
/// \return Feature expansion packet for F28-F21
constexpr auto make_feature_expansion_f28_f21_packet(Address::value_type addr,
                                                     uint8_t f28_f21) {
  return make_feature_expansion_f28_f21_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco}, f28_f21);
}

/// Make binary state short packet
///
/// \param  addr      Address
/// \param  dlllllll  Binary state low byte and number
/// \return Binary state short packet
constexpr auto make_binary_state_short_packet(Address addr, uint8_t dlllllll) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b1101'1101u;
  *last++ = dlllllll;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make binary state short packet
///
/// \param  addr      Address
/// \param  dlllllll  Binary state low byte and number
/// \return Binary state short packet
constexpr auto make_binary_state_short_packet(Address::value_type addr,
                                              uint8_t dlllllll) {
  return make_binary_state_short_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco},
    dlllllll);
}

/// Make binary state long packet
///
/// \param  addr      Address
/// \param  dlllllll  Binary state low byte and number
/// \param  hhhhhhhh  Binary state high byte
/// \return Binary state long packet
constexpr auto make_binary_state_long_packet(Address addr,
                                             uint8_t dlllllll,
                                             uint8_t hhhhhhhh) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b1100'0000u;
  *last++ = dlllllll;
  *last++ = hhhhhhhh;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make binary state long packet
///
/// \param  addr      Address
/// \param  dlllllll  Binary state low byte and number
/// \param  hhhhhhhh  Binary state high byte
/// \return Binary state long packet
constexpr auto make_binary_state_long_packet(Address::value_type addr,
                                             uint8_t dlllllll,
                                             uint8_t hhhhhhhh) {
  return make_binary_state_long_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco},
    dlllllll,
    hhhhhhhh);
}

/// Make CV access long form packet for verifying CV
///
/// \param  addr    Address
/// \param  cv_addr CV address
/// \param  byte    CV value
/// \return CV access long form packet for verifying CV
constexpr auto make_cv_access_long_verify_packet(Address addr,
                                                 uint32_t cv_addr,
                                                 uint8_t byte = 0u) {
  assert(addr.type == Address::Broadcast ||        //
         addr.type == Address::BasicLoco ||        //
         addr.type == Address::ExtendedLoco ||     //
         addr.type == Address::BasicAccessory ||   //
         addr.type == Address::ExtendedAccessory); //
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = static_cast<uint8_t>(0b1110'0100u | (cv_addr & 0x3FFu) >> 8u);
  *last++ = static_cast<uint8_t>(cv_addr);
  *last++ = byte;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make CV access long form packet for writing CV
///
/// \param  addr    Address
/// \param  cv_addr CV address
/// \param  byte    CV value
/// \return CV access long form packet for writing CV
constexpr auto
make_cv_access_long_write_packet(Address addr, uint32_t cv_addr, uint8_t byte) {
  assert(addr.type == Address::Broadcast ||        //
         addr.type == Address::BasicLoco ||        //
         addr.type == Address::ExtendedLoco ||     //
         addr.type == Address::BasicAccessory ||   //
         addr.type == Address::ExtendedAccessory); //
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = static_cast<uint8_t>(0b1110'1100u | (cv_addr & 0x3FFu) >> 8u);
  *last++ = static_cast<uint8_t>(cv_addr);
  *last++ = byte;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make CV access long form packet for verifying CV bit
///
/// \param  addr    Address
/// \param  cv_addr CV address
/// \param  bit     Bit
/// \param  pos     Bit position
/// \return CV access long form packet for verifying CV bit
constexpr auto make_cv_access_long_verify_packet(Address addr,
                                                 uint32_t cv_addr,
                                                 bool bit,
                                                 uint32_t pos) {
  assert(addr.type == Address::Broadcast ||        //
         addr.type == Address::BasicLoco ||        //
         addr.type == Address::ExtendedLoco ||     //
         addr.type == Address::BasicAccessory ||   //
         addr.type == Address::ExtendedAccessory); //
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = static_cast<uint8_t>(0b1110'1000u | (cv_addr & 0x3FFu) >> 8u);
  *last++ = static_cast<uint8_t>(cv_addr);
  auto const d{static_cast<uint32_t>(bit << 3u)};
  *last++ = static_cast<uint8_t>(0b1110'0000u | d | (pos & 0b111u));
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make CV access long form packet for writing CV bit
///
/// \param  addr    Address
/// \param  cv_addr CV address
/// \param  bit     Bit
/// \param  pos     Bit position
/// \return CV access long form packet for writing CV bit
constexpr auto make_cv_access_long_write_packet(Address addr,
                                                uint32_t cv_addr,
                                                bool bit,
                                                uint32_t pos) {
  assert(addr.type == Address::Broadcast ||        //
         addr.type == Address::BasicLoco ||        //
         addr.type == Address::ExtendedLoco ||     //
         addr.type == Address::BasicAccessory ||   //
         addr.type == Address::ExtendedAccessory); //
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = static_cast<uint8_t>(0b1110'1000u | (cv_addr & 0x3FFu) >> 8u);
  *last++ = static_cast<uint8_t>(cv_addr);
  auto const d{static_cast<uint32_t>(bit << 3u)};
  *last++ = static_cast<uint8_t>(0b1111'0000u | d | (pos & 0b111u));
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make CV access long form service packet for verifying CV
///
/// \param  cv_addr CV address
/// \param  byte    CV value
/// \return CV access long form service packet for verifying CV
constexpr auto make_cv_access_long_verify_service_packet(uint32_t cv_addr,
                                                         uint8_t byte) {
  Packet packet{};
  auto first{begin(packet)};
  auto last{first};
  *last++ = static_cast<uint8_t>(0b0111'0100u | (cv_addr & 0x3FFu) >> 8u);
  *last++ = static_cast<uint8_t>(cv_addr);
  *last++ = byte;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make CV access long form service packet for writing CV
///
/// \param  cv_addr CV address
/// \param  byte    CV value
/// \return CV access long form service packet for writing CV
constexpr auto make_cv_access_long_write_service_packet(uint32_t cv_addr,
                                                        uint8_t byte) {
  Packet packet{};
  auto first{begin(packet)};
  auto last{first};
  *last++ = static_cast<uint8_t>(0b0111'1100u | (cv_addr & 0x3FFu) >> 8u);
  *last++ = static_cast<uint8_t>(cv_addr);
  *last++ = byte;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make CV access long form service packet for verifying CV bit
///
/// \param  cv_addr CV address
/// \param  bit     Bit
/// \param  pos     Bit position
/// \return CV access long form service packet for verifying CV bit
constexpr auto make_cv_access_long_verify_service_packet(uint32_t cv_addr,
                                                         bool bit,
                                                         uint32_t pos) {
  Packet packet{};
  auto first{begin(packet)};
  auto last{first};
  *last++ = static_cast<uint8_t>(0b0111'1000u | (cv_addr & 0x3FFu) >> 8u);
  *last++ = static_cast<uint8_t>(cv_addr);
  auto const d{static_cast<uint32_t>(bit << 3u)};
  *last++ = static_cast<uint8_t>(0b1110'0000u | d | (pos & 0b111u));
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make CV access long form service packet for writing CV bit
///
/// \param  cv_addr CV address
/// \param  bit     Bit
/// \param  pos     Bit position
/// \return CV access long form service packet for writing CV bit
constexpr auto make_cv_access_long_write_service_packet(uint32_t cv_addr,
                                                        bool bit,
                                                        uint32_t pos) {
  Packet packet{};
  auto first{begin(packet)};
  auto last{first};
  *last++ = static_cast<uint8_t>(0b0111'1000u | (cv_addr & 0x3FFu) >> 8u);
  *last++ = static_cast<uint8_t>(cv_addr);
  auto const d{static_cast<uint32_t>(bit << 3u)};
  *last++ = static_cast<uint8_t>(0b1111'0000u | d | (pos & 0b111u));
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make CV access short form packet for writing CV
///
/// \param  addr    Address
/// \param  kkkk    CV identifier
/// \param  byte1   First CV value
/// \param  byte2   Second CV value
/// \return CV access long form packet for writing CV
constexpr auto make_cv_access_short_write_packet(Address addr,
                                                 uint8_t kkkk,
                                                 uint8_t byte1,
                                                 uint8_t byte2 = 0u) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  assert(kkkk == 0b0010u || // CV23
         kkkk == 0b0011u || // CV24
         kkkk == 0b0100u || // CV17/18
         kkkk == 0b0101u || // CV31/32
         kkkk == 0b0110u);  // CV19/20
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = static_cast<uint8_t>(0b1111'0000u | kkkk);
  *last++ = byte1;
  if (kkkk == 0b0100u || kkkk == 0b0101u || kkkk == 0b0110u) *last++ = byte2;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make CV access short form packet for writing CV
///
/// \param  addr    Address
/// \param  kkkk    CV identifier
/// \param  byte1   First CV value
/// \param  byte2   Second CV value
/// \return CV access long form packet for writing CV
constexpr auto make_cv_access_short_write_packet(Address::value_type addr,
                                                 uint8_t kkkk,
                                                 uint8_t byte1,
                                                 uint8_t byte2 = 0u) {
  return make_cv_access_short_write_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco},
    kkkk,
    byte1,
    byte2);
}

/// Make LOGON_ENABLE packet
///
/// \param  gg          Address group
/// \param  cid         Command station ID
/// \param  session_id  Session ID
/// \return LOGON_ENABLE packet
constexpr auto
make_logon_enable_packet(AddressGroup gg, uint16_t cid, uint8_t session_id) {
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address({254u, Address::AutomaticLogon}, first)};
  *last++ = static_cast<uint8_t>(0b1111'1100u | std::to_underlying(gg));
  last = uint16_2data(cid, last);
  *last++ = session_id;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make LOGON_SELECT packet
///
/// \param  manufacturer_id Manufacturer ID
/// \param  did             Unique ID
/// \return LOGON_SELECT packet
constexpr auto make_logon_select_packet(uint16_t manufacturer_id,
                                        uint32_t did,
                                        uint8_t subcommand = 0b1111'1111u) {
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address({254u, Address::AutomaticLogon}, first)};
  *last++ = static_cast<uint8_t>(0b1101'0000u | (manufacturer_id >> 8u));
  *last++ = static_cast<uint8_t>(manufacturer_id);
  last = uint32_2data(did, last);
  *last++ = subcommand;
  *last = dcc::crc8({first, last});
  ++last;
  *last = dcc::exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make LOGON_ASSIGN packet
///
/// \param  manufacturer_id Manufacturer ID
/// \param  did             Unique ID
/// \param  addr            Address (own encoding!)
/// \return LOGON_ASSIGN packet
constexpr auto make_logon_assign_packet(uint16_t manufacturer_id,
                                        uint32_t did,
                                        uint16_t addr) {
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address({254u, Address::AutomaticLogon}, first)};
  *last++ = static_cast<uint8_t>(0b1110'0000u | (manufacturer_id >> 8u));
  *last++ = static_cast<uint8_t>(manufacturer_id);
  last = uint32_2data(did, last);
  last = uint16_2data(addr, last);
  *last = dcc::crc8({first, last});
  ++last;
  *last = dcc::exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make basic accessory packet
///
/// \param  addr  Address
/// \param  r     Select pair of outputs
/// \param  d     State
/// \return Basic accessory packet
constexpr auto make_basic_accessory_packet(Address addr, bool r, bool d) {
  assert(addr.type == Address::BasicAccessory ||
         addr.type == Address::ExtendedAccessory);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *(first + 1) = static_cast<uint8_t>(*(first + 1) | (d << 3u) | (r << 0u));
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

} // namespace dcc
