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
#include <cstring>
#include <utility>
#include <ztl/math.hpp>
#include "address.hpp"
#include "crc8.hpp"
#include "exor.hpp"
#include "instruction.hpp"
#include "packet.hpp"

#if __has_include(<stdfloat>)
#  include <stdfloat>
#endif

namespace dcc {

/// Data to uint16_t
///
/// \tparam RandomIt  std::random_access_iterator
/// \param  first     Beginning of the source range
/// \return uint16_t
template<std::random_access_iterator RandomIt>
requires(sizeof(std::iter_value_t<RandomIt>) == 1uz)
constexpr auto data2uint16(RandomIt first) {
  return static_cast<uint16_t>(first[0uz] << 8u | first[1uz] << 0u);
}

/// uint16 to data
///
/// \tparam OutputIt  std::output_iterator
/// \param  hword     Half-word to convert
/// \param  out       Beginning of the destination range
/// \return Output iterator one past the last element copied
template<std::output_iterator<uint8_t> OutputIt>
constexpr auto uint16_2data(uint16_t hword, OutputIt out) {
  *out++ = static_cast<uint8_t>(hword >> 8u);
  *out++ = static_cast<uint8_t>(hword >> 0u);
  return out;
}

/// Data to uint32_t
///
/// \tparam RandomIt  std::random_access_iterator
/// \param  first     Beginning of the source range
/// \return uint32_t
template<std::random_access_iterator RandomIt>
requires(sizeof(std::iter_value_t<RandomIt>) == 1uz)
constexpr auto data2uint32(RandomIt first) {
  return static_cast<uint32_t>(first[0uz] << 24u | first[1uz] << 16u |
                               first[2uz] << 8u | first[3uz] << 0u);
}

/// uint32_t to data
///
/// \tparam OutputIt  std::output_iterator
/// \param  word      Word to convert
/// \param  out       Beginning of the destination range
/// \return Output iterator one past the last element copied
template<std::output_iterator<uint8_t> OutputIt>
constexpr auto uint32_2data(uint32_t word, OutputIt out) {
  *out++ = static_cast<uint8_t>(word >> 24u);
  *out++ = static_cast<uint8_t>(word >> 16u);
  *out++ = static_cast<uint8_t>(word >> 8u);
  *out++ = static_cast<uint8_t>(word >> 0u);
  return out;
}

/// Make decoder control - digital decoder reset packet
///
/// \param  addr  Address
/// \return Decoder control - digital decoder reset packet
constexpr auto make_reset_packet(Address addr) {
  assert(addr.type == Address::Broadcast || addr.type == Address::BasicLoco ||
         addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b0000'0000u;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make decoder control - digital decoder reset packet
///
/// \param  addr  Address
/// \return Decoder control - digital decoder reset packet
constexpr auto make_reset_packet(Address::value_type addr = 0u) {
  return make_reset_packet(
    {addr,
     !addr ? Address::Broadcast
           : (addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco)});
}

/// Make decoder control - hard reset packet
///
/// \param  addr  Address
/// \return Decoder control - hard reset packet
constexpr auto make_hard_reset_packet(Address addr) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b0000'0001u;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make decoder control - hard reset packet
///
/// \param  addr  Address
/// \return Decoder control - hard reset packet
constexpr auto make_hard_reset_packet(Address::value_type addr = 0u) {
  return make_hard_reset_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco});
}

/// Make decoder control - factory test packet
///
/// \param  addr  Address
/// \param  bit0  Bit0 of the instruction byte
/// \param  bytes Additional bytes to append
/// \return Decoder control - factory test packet
constexpr auto make_factory_test_packet(Address addr,
                                        bool bit0,
                                        std::span<uint8_t const> bytes = {}) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = static_cast<uint8_t>(0b0000'0010u | bit0);
  last = std::copy(cbegin(bytes), cend(bytes), last);
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make decoder control - factory test packet
///
/// \param  addr  Address
/// \param  bit0  Bit0 of the instruction byte
/// \param  bytes Additional bytes to append
/// \return Decoder control - factory test packet
constexpr auto make_factory_test_packet(Address::value_type addr,
                                        bool bit0,
                                        std::span<uint8_t const> bytes = {}) {
  return make_factory_test_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco},
    bit0,
    bytes);
}

/// Make decoder control - set advanced addressing packet
///
/// \param  addr    Address
/// \param  cv29_5  CV29:5
/// \return Decoder control - set advanced addressing packet
constexpr auto make_set_advanced_addressing_packet(Address addr, bool cv29_5) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = static_cast<uint8_t>(0b0000'1010u | cv29_5);
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make decoder control - set advanced addressing packet
///
/// \param  addr    Address
/// \param  cv29_5  CV29:5
/// \return Decoder control - set advanced addressing packet
constexpr auto make_set_advanced_addressing_packet(Address::value_type addr,
                                                   bool cv29_5) {
  return make_set_advanced_addressing_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco}, cv29_5);
}

/// Make decoder control - decoder acknowledgement request packet
///
/// \param  addr  Address
/// \return Decoder control - decoder acknowledgement request packet
constexpr auto make_ack_request_packet(Address addr) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b0000'1111u;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make decoder control - decoder acknowledgement request packet
///
/// \param  addr  Address
/// \return Decoder control - decoder acknowledgement request packet
constexpr auto make_ack_request_packet(Address::value_type addr) {
  return make_ack_request_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco});
}

/// Make consist control - set consist address packet
///
/// \param  addr  Address
/// \param  cv19  CV19
/// \return Consist control - set consist address packet
constexpr auto make_set_consist_address_packet(Address addr, uint8_t cv19) {
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

/// Make consist control - set consist address packet
///
/// \param  addr  Address
/// \param  cv19  CV19
/// \return Consist control - set consist address packet
constexpr auto make_set_consist_address_packet(Address::value_type addr,
                                               uint8_t cv19) {
  return make_set_consist_address_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco}, cv19);
}

/// Make advanced operations - speed, direction and functions packet
///
/// \tparam Fs...     Type of functions
/// \param  addr      Address
/// \param  rggggggg  Speed and direction byte
/// \param  fs...     Functions
/// \return Advanced operations - speed, direction and functions packet
template<std::unsigned_integral... Fs>
requires(sizeof...(Fs) > 0uz)
constexpr auto make_speed_direction_and_functions_packet(Address addr,
                                                         uint8_t rggggggg,
                                                         Fs... fs) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b0011'1100u;
  *last++ = rggggggg;
  ((*last++ = static_cast<uint8_t>(fs)), ...);
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make advanced operations - speed, direction and functions packet
///
/// \tparam Fs...     Type of functions
/// \param  addr      Address
/// \param  rggggggg  Speed and direction byte
/// \param  fs...     Functions
/// \return Advanced operations - speed, direction and functions packet
template<std::unsigned_integral... Fs>
requires(sizeof...(Fs) > 0uz)
constexpr auto make_speed_direction_and_functions_packet(
  Address::value_type addr, uint8_t rggggggg, Fs... fs) {
  return make_speed_direction_and_functions_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco},
    rggggggg,
    fs...);
}

/// Make advanced operations - analog function group packet
///
/// \param  addr      Address
/// \param  ssssssss  Channel
/// \param  dddddddd  Value
/// \return Advanced operations - analog function group packet
constexpr auto make_analog_function_group_packet(Address addr,
                                                 uint8_t ssssssss,
                                                 uint8_t dddddddd) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b0011'1101u;
  *last++ = ssssssss;
  *last++ = dddddddd;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make advanced operations - analog function group packet
///
/// \param  addr      Address
/// \param  ssssssss  Channel
/// \param  dddddddd  Value
/// \return Advanced operations - analog function group packet
constexpr auto make_analog_function_group_packet(Address::value_type addr,
                                                 uint8_t ssssssss,
                                                 uint8_t dddddddd) {
  return make_analog_function_group_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco},
    ssssssss,
    dddddddd);
}

enum struct Consist : uint8_t {
  NotPart = 0b00u,
  Leading = 0b10u,
  Middle = 0b01u,
  Read = 0b11u
};

/// Make advanced operations - special operating modes packet
///
/// \param  addr      Address
/// \param  cc        Position in a consist
/// \param  shunting  Shunting function
/// \param  west      West bit
/// \param  east      East bit
/// \param  man       MAN function
/// \return Advanced operations - special operating modes packet
constexpr auto make_special_operating_modes(
  Address addr, Consist cc, bool shunting, bool west, bool east, bool man) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b0011'1110u;
  *last++ = static_cast<uint8_t>(man << 7u | east << 6u | west << 5u |
                                 shunting << 4u | std::to_underlying(cc) << 2u);
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make advanced operations - special operating modes packet
///
/// \param  addr      Address
/// \param  cc        Position in a consist
/// \param  shunting  Shunting function
/// \param  west      West bit
/// \param  east      East bit
/// \param  man       MAN function
/// \return Advanced operations - special operating modes packet
constexpr auto make_special_operating_modes(Address::value_type addr,
                                            Consist cc,
                                            bool shunting,
                                            bool west,
                                            bool east,
                                            bool man) {
  return make_special_operating_modes(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco},
    cc,
    shunting,
    west,
    east,
    man);
}

/// Make advanced operations - 128 speed step control packet
///
/// \param  addr      Address
/// \param  rggggggg  Speed and direction byte
/// \return Advanced operations - 128 speed step control packet
constexpr auto make_128_speed_step_control_packet(Address addr,
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

/// Make advanced operations - 128 speed step control packet
///
/// \param  addr      Address
/// \param  rggggggg  Speed and direction byte
/// \return Advanced operations - 128 speed step control packet
constexpr auto make_128_speed_step_control_packet(Address::value_type addr,
                                                  uint8_t rggggggg) {
  return make_128_speed_step_control_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco},
    rggggggg);
}

/// Make speed and direction packet
///
/// \param  addr    Address
/// \param  rggggg  Speed and direction byte
/// \return Speed and direction packet
constexpr auto make_speed_and_direction_packet(Address addr, uint8_t rggggg) {
  assert(addr.type == Address::Broadcast || addr.type == Address::BasicLoco ||
         addr.type == Address::ExtendedLoco);
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
/// \param  rggggg  Speed and direction byte
/// \return Speed and direction packet
constexpr auto make_speed_and_direction_packet(Address::value_type addr,
                                               uint8_t rggggg) {
  return make_speed_and_direction_packet(
    {addr,
     !addr ? Address::Broadcast
           : (addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco)},
    rggggg);
}

/// Make function group - F0-F4 packet
///
/// \param  addr  Address
/// \param  state F4-F0 state
/// \return Function group - F0-F4 packet
constexpr auto make_f0_f4_packet(Address addr, uint8_t state) {
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

/// Make function group - F0-F4 packet
///
/// \param  addr  Address
/// \param  state F4-F0 state
/// \return Function group - F0-F4 packet
constexpr auto make_f0_f4_packet(Address::value_type addr, uint8_t state) {
  return make_f0_f4_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco}, state);
}

/// Make function group - F9-F12 packet
///
/// \param  addr  Address
/// \param  state F12-F9 state
/// \return Function group - F9-F12 packet
constexpr auto make_f9_f12_packet(Address addr, uint8_t state) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b1010'0000u | (state & 0xFu);
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make function group - F9-F12 packet
///
/// \param  addr  Address
/// \param  state F12-F9 state
/// \return Function group - F9-F12 packet
constexpr auto make_f9_f12_packet(Address::value_type addr, uint8_t state) {
  return make_f9_f12_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco}, state);
}

/// Make function group - F5-F8 packet
///
/// \param  addr  Address
/// \param  state F8-F5 state
/// \return Function group - F5-F8 packet
constexpr auto make_f5_f8_packet(Address addr, uint8_t state) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b1011'0000u | (state & 0xFu);
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make function group - F5-F8 packet
///
/// \param  addr  Address
/// \param  state F8-F5 state
/// \return Function group - F5-F8 packet
constexpr auto make_f5_f8_packet(Address::value_type addr, uint8_t state) {
  return make_f5_f8_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco}, state);
}

/// Make feature expansion - binary state long packet
///
/// \param  addr      Address
/// \param  bin_addr  Binary state address
/// \param  d         Binary state
/// \return Feature expansion - binary state long packet
constexpr auto
make_binary_state_long_packet(Address addr, uint16_t bin_addr, bool d) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b1100'0000u;
  *last++ =
    static_cast<uint8_t>(static_cast<uint32_t>(d << 7u) | (bin_addr & 0x7Fu));
  *last++ = static_cast<uint8_t>(bin_addr >> 7u);
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make feature expansion - binary state long packet
///
/// \param  addr      Address
/// \param  bin_addr  Binary state address
/// \param  d         Binary state
/// \return Feature expansion - binary state long packet
constexpr auto make_binary_state_long_packet(Address::value_type addr,
                                             uint16_t bin_addr,
                                             bool d) {
  return make_binary_state_long_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco},
    bin_addr,
    d);
}

/// Make feature expansion - time packet
///
/// \param  weekday Weekday
/// \param  hour    Hour
/// \param  min     Minute
/// \param  acc     Acceleration factor
/// \param  update  Abrupt update
/// \return Feature expansion - time packet
constexpr auto make_time_packet(
  uint8_t weekday, uint8_t hour, uint8_t min, uint8_t acc, bool update) {
  assert(weekday <= 7u && hour <= 23u && min <= 59 && acc <= 63u);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address({0u, Address::Broadcast}, first)};
  *last++ = 0b1100'0001u;
  *last++ = min;
  *last++ = static_cast<uint8_t>(weekday << 5u | hour);
  *last++ = static_cast<uint8_t>(update << 7u | acc);
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make feature expansion - date packet
///
/// \param  day   Day
/// \param  month Month
/// \param  year  Year
/// \return Feature expansion - date packet
constexpr auto make_date_packet(uint8_t day, uint8_t month, uint16_t year) {
  assert(day >= 1u && day <= 7u && month >= 1u && month <= 12u &&
         year <= 4095u);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address({0u, Address::Broadcast}, first)};
  *last++ = 0b1100'0001u;
  *last++ = static_cast<uint8_t>(0b0100'0000 | day);
  *last++ = static_cast<uint8_t>(month << 4u | year >> 8u);
  *last++ = static_cast<uint8_t>(year);
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

#if defined(__STDCPP_FLOAT16_T__)
/// Make feature expansion - time scale packet
///
/// \param  scale
/// \return Feature expansion - time scale packet
constexpr auto make_time_scale_packet(std::float16_t scale) {
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address({0u, Address::Broadcast}, first)};
  *last++ = 0b1100'0001u;
  *last++ = 0b1011'1111u;
  memcpy(&scale, last, sizeof(scale));
  last += sizeof(scale);
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}
#endif

/// Make feature expansion - system time packet
///
/// \param  ms  Time [ms]
/// \return Feature expansion - system time packet
constexpr auto make_system_time_packet(uint16_t ms) {
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address({0u, Address::Broadcast}, first)};
  *last++ = 0b1100'0010u;
  *last++ = static_cast<uint8_t>(ms >> 8u);
  *last++ = static_cast<uint8_t>(ms >> 0u);
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// \todo Command station bla
constexpr auto make_command_station_properties_identifier_packet() {
  Packet packet{};
  return packet;
}

/// Make feature expansion - F29-F36 packet
///
/// \param  addr  Address
/// \param  state F36-F29 state
/// \return Feature expansion - F29-F36 packet
constexpr auto make_f29_f36_packet(Address addr, uint8_t state) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b1101'1000u;
  *last++ = state;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make feature expansion - F29-F36 packet
///
/// \param  addr  Address
/// \param  state F36-F29 state
/// \return Feature expansion - F29-F36 packet
constexpr auto make_f29_f36_packet(Address::value_type addr, uint8_t state) {
  return make_f29_f36_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco}, state);
}

/// Make feature expansion - F37-F44 packet
///
/// \param  addr  Address
/// \param  state F44-F37 state
/// \return Feature expansion - F37-F44 packet
constexpr auto make_f37_f44_packet(Address addr, uint8_t state) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b1101'1001u;
  *last++ = state;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make feature expansion - F37-F44 packet
///
/// \param  addr  Address
/// \param  state F44-F37 state
/// \return Feature expansion - F37-F44 packet
constexpr auto make_f37_f44_packet(Address::value_type addr, uint8_t state) {
  return make_f37_f44_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco}, state);
}

/// Make feature expansion - F45-F52 packet
///
/// \param  addr  Address
/// \param  state F52-F45 state
/// \return Feature expansion - F45-F52 packet
constexpr auto make_f45_f52_packet(Address addr, uint8_t state) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b1101'1010u;
  *last++ = state;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make feature expansion - F45-F52 packet
///
/// \param  addr  Address
/// \param  state F52-F45 state
/// \return Feature expansion - F45-F52 packet
constexpr auto make_f45_f52_packet(Address::value_type addr, uint8_t state) {
  return make_f45_f52_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco}, state);
}

/// Make feature expansion - F53-F60 packet
///
/// \param  addr  Address
/// \param  state F60-F53 state
/// \return Feature expansion - F53-F60 packet
constexpr auto make_f53_f60_packet(Address addr, uint8_t state) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b1101'1011u;
  *last++ = state;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make feature expansion - F53-F60 packet
///
/// \param  addr  Address
/// \param  state F60-F53 state
/// \return Feature expansion - F53-F60 packet
constexpr auto make_f53_f60_packet(Address::value_type addr, uint8_t state) {
  return make_f53_f60_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco}, state);
}

/// Make feature expansion - F61-F68 packet
///
/// \param  addr  Address
/// \param  state F68-F61 state
/// \return Feature expansion - F61-F68 packet
constexpr auto make_f61_f68_packet(Address addr, uint8_t state) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b1101'1100u;
  *last++ = state;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make feature expansion - F61-F68 packet
///
/// \param  addr  Address
/// \param  state F68-F61 state
/// \return Feature expansion - F61-F68 packet
constexpr auto make_f61_f68_packet(Address::value_type addr, uint8_t state) {
  return make_f61_f68_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco}, state);
}

/// Make feature expansion - binary state short packet
///
/// \param  addr      Address
/// \param  bin_addr  Binary state address
/// \param  d         Binary state
/// \return Feature expansion - binary state short packet
constexpr auto
make_binary_state_short_packet(Address addr, uint8_t bin_addr, bool d) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b1101'1101u;
  *last++ =
    static_cast<uint8_t>(static_cast<uint32_t>(d << 7u) | (bin_addr & 0x7Fu));
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make feature expansion - binary state short packet
///
/// \param  addr      Address
/// \param  bin_addr  Binary state address
/// \param  d         Binary state
/// \return Feature expansion - binary state short packet
constexpr auto make_binary_state_short_packet(Address::value_type addr,
                                              uint8_t bin_addr,
                                              bool d) {
  return make_binary_state_short_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco},
    bin_addr,
    d);
}

/// Make feature expansion - F13-20 packet
///
/// \param  addr  Address
/// \param  state F13-20 state
/// \return Feature expansion - F13-20 packet
constexpr auto make_f13_f20_packet(Address addr, uint8_t state) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b1101'1110u;
  *last++ = state;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make feature expansion - F13-20 packet
///
/// \param  addr  Address
/// \param  state F13-20 state
/// \return Feature expansion - F13-20 packet
constexpr auto make_f13_f20_packet(Address::value_type addr, uint8_t state) {
  return make_f13_f20_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco}, state);
}

/// Make feature expansion - F21-28 packet
///
/// \param  addr  Address
/// \param  state F21-28 state
/// \return Feature expansion - F21-28 packet
constexpr auto make_f21_f28_packet(Address addr, uint8_t state) {
  assert(addr.type == Address::BasicLoco || addr.type == Address::ExtendedLoco);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *last++ = 0b1101'1111u;
  *last++ = state;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make feature expansion - F21-28 packet
///
/// \param  addr  Address
/// \param  state F21-28 state
/// \return Feature expansion - F21-28 packet
constexpr auto make_f21_f28_packet(Address::value_type addr, uint8_t state) {
  return make_f21_f28_packet(
    {addr, addr <= 127u ? Address::BasicLoco : Address::ExtendedLoco}, state);
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
  assert(cv_addr < smath::pow(2u, 10u));
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
  assert(cv_addr < smath::pow(2u, 10u));
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
  assert(cv_addr < smath::pow(2u, 10u));
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
  assert(cv_addr < smath::pow(2u, 10u));
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
  assert(cv_addr < smath::pow(2u, 10u));
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
  assert(cv_addr < smath::pow(2u, 10u));
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
  assert(cv_addr < smath::pow(2u, 10u));
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
  assert(cv_addr < smath::pow(2u, 10u));
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

/// Logon group (RCN-218)
enum struct LogonGroup : uint8_t {
  All = 0b00u,
  Loco = 0b01u,
  Acc = 0b10u,
  Now = 0b11u
};

/// Make LOGON_ENABLE packet
///
/// \param  gg          Logon group
/// \param  cid         Command station ID
/// \param  session_id  Session ID
/// \return LOGON_ENABLE packet
constexpr auto
make_logon_enable_packet(LogonGroup gg, uint16_t cid, uint8_t session_id) {
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

/// Logon binding behavior (RCN-218)
enum struct LogonBindingBehavior : uint8_t {
  Reserved = 0b00u | 0b01u,
  Permanent = 0b10u,
  Temporary = 0b11u,
};

/// Make LOGON_ASSIGN packet
///
/// \param  manufacturer_id Manufacturer ID
/// \param  did             Unique ID
/// \param  addr            Address
/// \param  bb              Logon binding behavior
/// \return LOGON_ASSIGN packet
constexpr auto make_logon_assign_packet(
  uint16_t manufacturer_id,
  uint32_t did,
  Address addr,
  LogonBindingBehavior bb = LogonBindingBehavior::Temporary) {
  assert(manufacturer_id < smath::pow(2u, 12u));
  assert(addr.type == Address::BasicLoco ||        //
         addr.type == Address::ExtendedLoco ||     //
         addr.type == Address::BasicAccessory ||   //
         addr.type == Address::ExtendedAccessory); //
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address({254u, Address::AutomaticLogon}, first)};
  *last++ = static_cast<uint8_t>(0b1110'0000u | (manufacturer_id >> 8u));
  *last++ = static_cast<uint8_t>(manufacturer_id);
  last = uint32_2data(did, last);
  switch (addr.type) {
    case Address::BasicLoco:
      last = encode_address(addr, ++last);
      packet[7uz] = static_cast<uint8_t>(std::to_underlying(bb) << 6u) | 0x38u;
      break;
    case Address::ExtendedLoco:
      last = encode_address(addr, last);
      packet[7uz] =
        static_cast<uint8_t>(std::to_underlying(bb) << 6u) | packet[7uz];
      break;
    case Address::BasicAccessory: assert(false); break;
    case Address::ExtendedAccessory: assert(false); break;
    default: assert(false); break;
  }
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
  assert(addr.type == Address::BasicAccessory);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  *(first + 1) = static_cast<uint8_t>(*(first + 1) | (d << 3u) | (r << 0u));
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make basic accessory packet
///
/// \param  addr  Address
/// \param  r     Select pair of outputs
/// \param  d     State
/// \return Basic accessory packet
constexpr auto
make_basic_accessory_packet(Address::value_type addr, bool r, bool d) {
  return make_basic_accessory_packet({addr, Address::BasicAccessory}, r, d);
}

/// Make accessory NOP packet
///
/// \param  addr  Address
/// \return Accessory NOP packet
constexpr auto make_accessory_nop_packet(Address addr) {
  assert(addr.type == Address::BasicAccessory ||
         addr.type == Address::ExtendedAccessory);
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address(addr, first)};
  packet[1uz] = static_cast<uint8_t>((packet[1uz] & 0b0111'0110) | 0b000'1000 |
                                     (addr.type == Address::ExtendedAccessory));
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

/// Make digital decoder - idle packet
///
/// \return Digital decoder - idle packet
consteval auto make_idle_packet() {
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address({255u, Address::Idle}, first)};
  *last++ = 0b0000'0000;
  *last = exor({first, last});
  packet.resize(static_cast<Packet::size_type>(++last - first));
  return packet;
}

} // namespace dcc
