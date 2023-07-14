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
  using size_type = ztl::smallest_unsigned_t<DCC_MAX_PACKET_SIZE>;
  using difference_type = std::ptrdiff_t;
  using reference = value_type&;
  using const_reference = value_type const&;
  using pointer = value_type*;
  using const_pointer = value_type const*;
  using iterator = std::array<value_type, DCC_MAX_PACKET_SIZE>::iterator;
  using const_iterator =
    std::array<value_type, DCC_MAX_PACKET_SIZE>::const_iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  // Implicitly-defined member functions
  constexpr Packet() = default;

  template<std::unsigned_integral... Ts>
  constexpr Packet(Ts&&... ts) requires(sizeof...(Ts) <= DCC_MAX_PACKET_SIZE)
    : _data{static_cast<value_type>(ts)...}, _size{sizeof...(Ts)} {}

  // Element access
  constexpr reference at(size_type pos) { return _data.at(pos); }
  constexpr const_reference at(size_type pos) const { return at(pos); }

  constexpr reference operator[](size_type pos) { return _data[pos]; }
  constexpr const_reference operator[](size_type pos) const {
    return _data[pos];
  }

  constexpr reference front() { return _data[0uz]; }
  constexpr const_reference front() const { return _data[0uz]; }

  constexpr reference back() { return size() ? _data[_size - 1uz] : front(); }
  constexpr const_reference back() const {
    return size() ? _data[_size - 1uz] : front();
  }

  constexpr pointer data() { return std::data(_data); }
  constexpr const_pointer data() const { return std::data(_data); }

  // Iterators
  constexpr iterator begin() { return std::begin(_data); }
  constexpr const_iterator begin() const { return std::begin(_data); }
  constexpr const_iterator cbegin() const { return begin(); }

  constexpr iterator end() { return std::begin(_data) + size(); }
  constexpr const_iterator end() const { return std::begin(_data) + size(); }
  constexpr const_iterator cend() const { return end(); }

  constexpr reverse_iterator rbegin() { return reverse_iterator{end()}; }
  constexpr const_reverse_iterator rbegin() const {
    return const_reverse_iterator{end()};
  }
  constexpr const_reverse_iterator crbegin() const { return rbegin(); }

  constexpr reverse_iterator rend() { return reverse_iterator{begin()}; }
  constexpr const_reverse_iterator rend() const {
    return const_reverse_iterator{begin()};
  }
  constexpr const_reverse_iterator crend() const { return rend(); }

  // Capacity
  constexpr bool empty() const { return size() == 0uz; }

  constexpr size_type& size() { return _size; }
  constexpr size_type const& size() const { return _size; }

  constexpr size_type max_size() const { return DCC_MAX_PACKET_SIZE; }

  // Modifiers
  constexpr void clear() { size() = 0uz; }

  // Non-member functions
  friend constexpr bool operator==(Packet const&, Packet const&) = default;

private:
  std::array<value_type, DCC_MAX_PACKET_SIZE> _data{};
  ztl::smallest_unsigned_t<DCC_MAX_PACKET_SIZE> _size{};
};

constexpr auto data(Packet& p) -> decltype(p.data()) { return p.data(); }
constexpr auto data(Packet const& p) -> decltype(p.data()) { return p.data(); }

[[nodiscard]] constexpr auto empty(Packet const& p) -> decltype(p.empty()) {
  return p.empty();
}

constexpr auto size(Packet& p) -> decltype(p.size()) { return p.size(); }
constexpr auto size(Packet const& p) -> decltype(p.size()) { return p.size(); }

constexpr auto begin(Packet& p) -> decltype(p.begin()) { return p.begin(); }
constexpr auto begin(Packet const& p) -> decltype(p.begin()) {
  return p.begin();
}
constexpr auto cbegin(Packet const& p) -> decltype(p.begin()) {
  return p.begin();
}

constexpr auto end(Packet& p) -> decltype(p.end()) { return p.end(); }
constexpr auto end(Packet const& p) -> decltype(p.end()) { return p.end(); }
constexpr auto cend(Packet const& p) -> decltype(p.end()) { return p.end(); }

constexpr auto rbegin(Packet& p) -> decltype(p.rbegin()) { return p.rbegin(); }
constexpr auto rbegin(Packet const& p) -> decltype(p.rbegin()) {
  return p.rbegin();
}
constexpr auto crbegin(Packet const& p) -> decltype(p.rbegin()) {
  return p.rbegin();
}

constexpr auto rend(Packet& p) -> decltype(p.rend()) { return p.rend(); }
constexpr auto rend(Packet const& p) -> decltype(p.rend()) { return p.rend(); }
constexpr auto crend(Packet const& p) -> decltype(p.rend()) { return p.rend(); }

/// Make an idle packet
///
/// \return Idle packet
consteval auto make_idle_packet() { return Packet{0xFFu, 0x00u, 0xFFu}; }

/// Make a reset packet
///
/// \return Reset packet
consteval auto make_reset_packet() { return Packet{0x00u, 0x00u, 0x00u}; }

/// Make an advanced operations speed packet
///
/// \param  addr  Address
/// \param  dir   Direction
/// \param  speed Speed (0-126)
/// \return Advanced operations speed packet
constexpr auto make_advanced_operations_speed_packet(Address::value_type addr,
                                                     int8_t dir,
                                                     uint8_t speed) {
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address({addr, addr < 128u ? Address::Short : Address::Long},
                           first)};
  *last++ = 0b0011'1111u;
  *last++ = (dir > 0) << 7u | (speed & 0x7Fu);
  *last = exor({first, last});
  packet.size() = ++last - first;
  return packet;
}

/// TODO
constexpr auto
make_advanced_operations_restricted_speed_packet(Address::value_type addr) {
  Packet packet{};
  return packet;
}

/// TODO
constexpr auto make_advanced_operations_analog_function_group_packet(
  Address::value_type addr) {
  Packet packet{};
  return packet;
}

/// TODO
constexpr auto make_speed_and_direction_packet(Address::value_type addr) {
  Packet packet{};
  return packet;
}

/// Make a function group packet for F4-F0
///
/// \param  addr  Address
/// \param  state F4-F0 state
/// \return Function group packet for F4-0
constexpr auto make_function_group_f4_f0_packet(Address::value_type addr,
                                                uint8_t state) {
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address({addr, addr < 128u ? Address::Short : Address::Long},
                           first)};
  *last++ = 0b1000'0000u | (state & 0b1u) << 4u | (state & 0x1Fu) >> 1u;
  *last = exor({first, last});
  packet.size() = ++last - first;
  return packet;
}

/// Make a function group packet for F8-F5
///
/// \param  addr  Address
/// \param  state F8-F5 state
/// \return Function group packet for F8-F5
constexpr auto make_function_group_f8_f5_packet(Address::value_type addr,
                                                uint8_t state) {
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address({addr, addr < 128u ? Address::Short : Address::Long},
                           first)};
  *last++ = 0b1011'0000u | (state & 0xFu);
  *last = exor({first, last});
  packet.size() = ++last - first;
  return packet;
}

/// Make a function group packet for F12-F9
///
/// \param  addr  Address
/// \param  state F12-F9 state
/// \return Function group packet for F12-F9
constexpr auto make_function_group_f12_f9_packet(Address::value_type addr,
                                                 uint8_t state) {
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address({addr, addr < 128u ? Address::Short : Address::Long},
                           first)};
  *last++ = 0b1010'0000u | (state & 0xFu);
  *last = exor({first, last});
  packet.size() = ++last - first;
  return packet;
}

/// Make a feature expansion packet for F20-F13
///
/// \param  addr  Address
/// \param  state F20-F13 state
/// \return Feature expansion packet for F20-F13
constexpr auto make_feature_expansion_f20_f13_packet(Address::value_type addr,
                                                     uint8_t state) {
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address({addr, addr < 128u ? Address::Short : Address::Long},
                           first)};
  *last++ = 0b1101'1110u;
  *last++ = state;
  *last = exor({first, last});
  packet.size() = ++last - first;
  return packet;
}

/// Make a feature expansion packet for F28-F21
///
/// \param  addr  Address
/// \param  state F28-F21 state
/// \return Feature expansion packet for F28-F21
constexpr auto make_feature_expansion_f28_f21_packet(Address::value_type addr,
                                                     uint8_t state) {
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address({addr, addr < 128u ? Address::Short : Address::Long},
                           first)};
  *last++ = 0b1101'1111u;
  *last++ = state;
  *last = exor({first, last});
  packet.size() = ++last - first;
  return packet;
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
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address({addr, addr < 128u ? Address::Short : Address::Long},
                           first)};
  *last++ = 0b1110'0100u | (cv_addr & 0x3FFu) >> 8u;
  *last++ = static_cast<uint8_t>(cv_addr);
  *last++ = byte;
  *last = exor({first, last});
  packet.size() = ++last - first;
  return packet;
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
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address({addr, addr < 128u ? Address::Short : Address::Long},
                           first)};
  *last++ = 0b1110'1100u | (cv_addr & 0x3FFu) >> 8u;
  *last++ = static_cast<uint8_t>(cv_addr);
  *last++ = byte;
  *last = exor({first, last});
  packet.size() = ++last - first;
  return packet;
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
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address({addr, addr < 128u ? Address::Short : Address::Long},
                           first)};
  *last++ = 0b1110'1000u | (cv_addr & 0x3FFu) >> 8u;
  *last++ = static_cast<uint8_t>(cv_addr);
  *last++ = 0b1110'0000u | bit << 3u | (pos & 0b111u);
  *last = exor({first, last});
  packet.size() = ++last - first;
  return packet;
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
  Packet packet{};
  auto first{begin(packet)};
  auto last{encode_address({addr, addr < 128u ? Address::Short : Address::Long},
                           first)};
  *last++ = 0b1110'1000u | (cv_addr & 0x3FFu) >> 8u;
  *last++ = static_cast<uint8_t>(cv_addr);
  *last++ = 0b1111'0000u | bit << 3u | (pos & 0b111u);
  *last = exor({first, last});
  packet.size() = ++last - first;
  return packet;
}

/// Make a CV access long form service packet for verifying CV
///
/// \param  cv_addr CV address
/// \param  byte    CV value
/// \return CV access long form service packet for verifying CV
constexpr auto make_cv_access_long_verify_service_packet(uint32_t cv_addr,
                                                         uint8_t byte) {
  Packet packet{};
  auto first{begin(packet)};
  auto last{first};
  *last++ = 0b0111'0100u | (cv_addr & 0x3FFu) >> 8u;
  *last++ = static_cast<uint8_t>(cv_addr);
  *last++ = byte;
  *last = exor({first, last});
  packet.size() = ++last - first;
  return packet;
}

/// Make a CV access long form service packet for writing CV
///
/// \param  cv_addr CV address
/// \param  byte    CV value
/// \return CV access long form service packet for writing CV
constexpr auto make_cv_access_long_write_service_packet(uint32_t cv_addr,
                                                        uint8_t byte) {
  Packet packet{};
  auto first{begin(packet)};
  auto last{first};
  *last++ = 0b0111'1100u | (cv_addr & 0x3FFu) >> 8u;
  *last++ = static_cast<uint8_t>(cv_addr);
  *last++ = byte;
  *last = exor({first, last});
  packet.size() = ++last - first;
  return packet;
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
  Packet packet{};
  auto first{begin(packet)};
  auto last{first};
  *last++ = 0b0111'1000u | (cv_addr & 0x3FFu) >> 8u;
  *last++ = static_cast<uint8_t>(cv_addr);
  *last++ = 0b1110'0000u | bit << 3u | (pos & 0b111u);
  *last = exor({first, last});
  packet.size() = ++last - first;
  return packet;
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
  Packet packet{};
  auto first{begin(packet)};
  auto last{first};
  *last++ = 0b0111'1000u | (cv_addr & 0x3FFu) >> 8u;
  *last++ = static_cast<uint8_t>(cv_addr);
  *last++ = 0b1111'0000u | bit << 3u | (pos & 0b111u);
  *last = exor({first, last});
  packet.size() = ++last - first;
  return packet;
}

}  // namespace dcc
