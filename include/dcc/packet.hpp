// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Packet
///
/// \file   dcc/packet.hpp
/// \author Vincent Hamp
/// \date   31/01/2022

#pragma once

#include <array>
#include <cstdint>
#include <ztl/limits.hpp>
#include "exor.hpp"

namespace dcc {

struct Packet {
  using value_type = uint8_t;
  constexpr bool operator==(Packet const&) const = default;
  std::array<value_type, DCC_MAX_PACKET_SIZE> data{};
  ztl::smallest_unsigned_t<DCC_MAX_PACKET_SIZE> size{};
};

/// Make an idle packet
///
/// \return Idle packet
consteval auto make_idle_packet() {
  return Packet{.data = {0xFFu, 0x00u, 0xFFu}, .size = 3uz};
}

/// Make a reset packet
///
/// \return Reset packet
consteval auto make_reset_packet() {
  return Packet{.data = {0x00u, 0x00u, 0x00u}, .size = 3uz};
}

/// Make an advanced operations speed packet
///
/// \param  addr  Address
/// \param  dir   Direction
/// \param  speed Speed (0-126)
/// \return Advanced operations speed packet
constexpr auto make_advanced_operations_speed_packet(Address::value_type addr,
                                                     int8_t dir,
                                                     uint8_t speed) {
  Packet retval{};
  auto first{begin(retval.data)};
  auto last{encode_address({addr, addr < 128u ? Address::Short : Address::Long},
                           first)};
  *last++ = 0b0011'1111u;
  *last++ = (dir > 0) << 7u | (speed & 0x7Fu);
  *last++ = exor({first, last});
  retval.size = last - first;
  return retval;
}

/// TODO
constexpr auto
make_advanced_operations_restricted_speed_packet(Address::value_type addr) {
  Packet retval{};
  return retval;
}

/// TODO
constexpr auto make_advanced_operations_analog_function_group_packet(
  Address::value_type addr) {
  Packet retval{};
  return retval;
}

/// TODO
constexpr auto make_speed_and_direction_packet(Address::value_type addr) {
  Packet retval{};
  return retval;
}

/// Make a function group packet for F4-F0
///
/// \param  addr  Address
/// \param  state F4-F0 state
/// \return Function group packet for F4-0
constexpr auto make_function_group_f4_f0_packet(Address::value_type addr,
                                                uint8_t state) {
  Packet retval{};
  auto first{begin(retval.data)};
  auto last{encode_address({addr, addr < 128u ? Address::Short : Address::Long},
                           first)};
  *last++ = 0b1000'0000u | (state & 0b1u) << 4u | (state & 0x1Fu) >> 1u;
  *last++ = exor({first, last});
  retval.size = last - first;
  return retval;
}

/// Make a function group packet for F8-F5
///
/// \param  addr  Address
/// \param  state F8-F5 state
/// \return Function group packet for F8-F5
constexpr auto make_function_group_f8_f5_packet(Address::value_type addr,
                                                uint8_t state) {
  Packet retval{};
  auto first{begin(retval.data)};
  auto last{encode_address({addr, addr < 128u ? Address::Short : Address::Long},
                           first)};
  *last++ = 0b1011'0000u | (state & 0xFu);
  *last++ = exor({first, last});
  retval.size = last - first;
  return retval;
}

/// Make a function group packet for F12-F9
///
/// \param  addr  Address
/// \param  state F12-F9 state
/// \return Function group packet for F12-F9
constexpr auto make_function_group_f12_f9_packet(Address::value_type addr,
                                                 uint8_t state) {
  Packet retval{};
  auto first{begin(retval.data)};
  auto last{encode_address({addr, addr < 128u ? Address::Short : Address::Long},
                           first)};
  *last++ = 0b1010'0000u | (state & 0xFu);
  *last++ = exor({first, last});
  retval.size = last - first;
  return retval;
}

/// Make a feature expansion packet for F20-F13
///
/// \param  addr  Address
/// \param  state F20-F13 state
/// \return Feature expansion packet for F20-F13
constexpr auto make_feature_expansion_f20_f13_packet(Address::value_type addr,
                                                     uint8_t state) {
  Packet retval{};
  auto first{begin(retval.data)};
  auto last{encode_address({addr, addr < 128u ? Address::Short : Address::Long},
                           first)};
  *last++ = 0b1101'1110u;
  *last++ = state;
  *last++ = exor({first, last});
  retval.size = last - first;
  return retval;
}

/// Make a feature expansion packet for F28-F21
///
/// \param  addr  Address
/// \param  state F28-F21 state
/// \return Feature expansion packet for F28-F21
constexpr auto make_feature_expansion_f28_f21_packet(Address::value_type addr,
                                                     uint8_t state) {
  Packet retval{};
  auto first{begin(retval.data)};
  auto last{encode_address({addr, addr < 128u ? Address::Short : Address::Long},
                           first)};
  *last++ = 0b1101'1111u;
  *last++ = state;
  *last++ = exor({first, last});
  retval.size = last - first;
  return retval;
}

/// Make a CV access long form packet for verifying CV
///
/// \param  addr    Address
/// \param  cv_addr CV address
/// \param  byte    CV value
/// \return CV access long form packet for verifying CV
constexpr auto make_cv_access_long_verify_packet(Address::value_type addr,
                                                 uint32_t cv_addr,
                                                 uint8_t byte = 0u) {
  Packet retval{};
  auto first{begin(retval.data)};
  auto last{encode_address({addr, addr < 128u ? Address::Short : Address::Long},
                           first)};
  *last++ = 0b1110'0100u | (cv_addr & 0x3FFu) >> 8u;
  *last++ = static_cast<uint8_t>(cv_addr);
  *last++ = byte;
  *last++ = exor({first, last});
  retval.size = last - first;
  return retval;
}

/// Make a CV access long form packet for writing CV
///
/// \param  addr    Address
/// \param  cv_addr CV address
/// \param  byte    CV value
/// \return CV access long form packet for writing CV
constexpr auto make_cv_access_long_write_packet(Address::value_type addr,
                                                uint32_t cv_addr,
                                                uint8_t byte) {
  Packet retval{};
  auto first{begin(retval.data)};
  auto last{encode_address({addr, addr < 128u ? Address::Short : Address::Long},
                           first)};
  *last++ = 0b1110'1100u | (cv_addr & 0x3FFu) >> 8u;
  *last++ = static_cast<uint8_t>(cv_addr);
  *last++ = byte;
  *last++ = exor({first, last});
  retval.size = last - first;
  return retval;
}

/// Make a CV access long form packet for verifying CV bit
///
/// \param  addr    Address
/// \param  cv_addr CV address
/// \param  bit     Bit
/// \param  pos     Bit position
/// \return CV access long form packet for verifying CV bit
constexpr auto make_cv_access_long_verify_packet(Address::value_type addr,
                                                 uint32_t cv_addr,
                                                 bool bit,
                                                 uint32_t pos) {
  Packet retval{};
  auto first{begin(retval.data)};
  auto last{encode_address({addr, addr < 128u ? Address::Short : Address::Long},
                           first)};
  *last++ = 0b1110'1000u | (cv_addr & 0x3FFu) >> 8u;
  *last++ = static_cast<uint8_t>(cv_addr);
  *last++ = 0b1110'0000u | bit << 3u | (pos & 0b111u);
  *last++ = exor({first, last});
  retval.size = last - first;
  return retval;
}

/// Make a CV access long form packet for writing CV bit
///
/// \param  addr    Address
/// \param  cv_addr CV address
/// \param  bit     Bit
/// \param  pos     Bit position
/// \return CV access long form packet for writing CV bit
constexpr auto make_cv_access_long_write_packet(Address::value_type addr,
                                                uint32_t cv_addr,
                                                bool bit,
                                                uint32_t pos) {
  Packet retval{};
  auto first{begin(retval.data)};
  auto last{encode_address({addr, addr < 128u ? Address::Short : Address::Long},
                           first)};
  *last++ = 0b1110'1000u | (cv_addr & 0x3FFu) >> 8u;
  *last++ = static_cast<uint8_t>(cv_addr);
  *last++ = 0b1111'0000u | bit << 3u | (pos & 0b111u);
  *last++ = exor({first, last});
  retval.size = last - first;
  return retval;
}

/// Make a CV access long form service packet for verifying CV
///
/// \param  cv_addr CV address
/// \param  byte    CV value
/// \return CV access long form service packet for verifying CV
constexpr auto make_cv_access_long_verify_service_packet(uint32_t cv_addr,
                                                         uint8_t byte) {
  Packet retval{};
  auto first{begin(retval.data)};
  auto last{first};
  *last++ = 0b0111'0100u | (cv_addr & 0x3FFu) >> 8u;
  *last++ = static_cast<uint8_t>(cv_addr);
  *last++ = byte;
  *last++ = exor({first, last});
  retval.size = last - first;
  return retval;
}

/// Make a CV access long form service packet for writing CV
///
/// \param  cv_addr CV address
/// \param  byte    CV value
/// \return CV access long form service packet for writing CV
constexpr auto make_cv_access_long_write_service_packet(uint32_t cv_addr,
                                                        uint8_t byte) {
  Packet retval{};
  auto first{begin(retval.data)};
  auto last{first};
  *last++ = 0b0111'1100u | (cv_addr & 0x3FFu) >> 8u;
  *last++ = static_cast<uint8_t>(cv_addr);
  *last++ = byte;
  *last++ = exor({first, last});
  retval.size = last - first;
  return retval;
}

/// Make a CV access long form service packet for verifying CV bit
///
/// \param  cv_addr CV address
/// \param  bit     Bit
/// \param  pos     Bit position
/// \return CV access long form service packet for verifying CV bit
constexpr auto make_cv_access_long_verify_service_packet(uint32_t cv_addr,
                                                         bool bit,
                                                         uint32_t pos) {
  Packet retval{};
  auto first{begin(retval.data)};
  auto last{first};
  *last++ = 0b0111'1000u | (cv_addr & 0x3FFu) >> 8u;
  *last++ = static_cast<uint8_t>(cv_addr);
  *last++ = 0b1110'0000u | bit << 3u | (pos & 0b111u);
  *last++ = exor({first, last});
  retval.size = last - first;
  return retval;
}

/// Make a CV access long form service packet for writing CV bit
///
/// \param  cv_addr CV address
/// \param  bit     Bit
/// \param  pos     Bit position
/// \return CV access long form service packet for writing CV bit
constexpr auto make_cv_access_long_write_service_packet(uint32_t cv_addr,
                                                        bool bit,
                                                        uint32_t pos) {
  Packet retval{};
  auto first{begin(retval.data)};
  auto last{first};
  *last++ = 0b0111'1000u | (cv_addr & 0x3FFu) >> 8u;
  *last++ = static_cast<uint8_t>(cv_addr);
  *last++ = 0b1111'0000u | bit << 3u | (pos & 0b111u);
  *last++ = exor({first, last});
  retval.size = last - first;
  return retval;
}

}  // namespace dcc