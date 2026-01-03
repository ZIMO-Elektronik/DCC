// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Address
///
/// \file   dcc/address.hpp
/// \author Vincent Hamp
/// \date   04/01/2022

#pragma once

#include <cassert>
#include <cstdint>
#include <iterator>
#include <span>
#include "packet.hpp"

namespace dcc {

/// Address value and type
struct Address {
  using value_type = uint16_t;

  constexpr Address& operator=(value_type const& v) {
    value = v;
    return *this;
  }

  friend constexpr bool operator==(Address const& lhs, Address const& rhs) {
    return (lhs.type == BasicLoco || lhs.type == ExtendedLoco) &&
               (rhs.type == BasicLoco || rhs.type == ExtendedLoco)
             ? lhs.value == rhs.value
             : lhs.value == rhs.value && lhs.type == rhs.type;
  }

  constexpr operator value_type&() { return value; }
  constexpr operator value_type const&() const { return value; }

  value_type value{};

  enum Type : uint8_t {
    UnknownService,    ///< Unknown or service (=no address)
    Broadcast,         ///< Broadcast
    BasicLoco,         ///< Basic loco (7 bit)
    BasicAccessory,    ///< Basic accessory (11 bit)
    ExtendedAccessory, ///< Extended accessory (11 bit)
    ExtendedLoco,      ///< Extended loco (14 bit)
    Reserved,          ///< Reserved
    DataTransfer,      ///< Data transfer
    AutomaticLogon,    ///< Automatic logon
    Idle               ///< Idle
  } type{};

  bool reversed{}; /// Direction reversed
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
/// Decode address
///
/// \tparam InputIt std::input_iterator
/// \param  first   Beginning of the range to decode from
/// \return Address
template<std::input_iterator InputIt>
requires(sizeof(std::iter_value_t<InputIt>) == 1uz)
constexpr Address decode_address(InputIt first) {
  // 0
  if (*first == 0u) return {*first, Address::Broadcast};
  // 1-127
  else if (*first <= 127u) return {*first, Address::BasicLoco};
  // 128-191
  else if (*first <= 191u) {
    auto const a7_2{(*first++ & 0x3Fu) << 2u};
    auto const a10_8{(~*first & 0x70u) << 4u};
    auto const a1_0{(*first >> 1u) & 0x03u};
    return {static_cast<Address::value_type>(a10_8 | a7_2 | a1_0),
            (*first & 0b1000'0000u) || !(*first & 0b1000'0001u)
              ? Address::BasicAccessory
              : Address::ExtendedAccessory};
  }
  // 192-231
  else if (*first <= 231u) {
    auto const a13_8{(*first++ & 0b0011'1111u) << 8u};
    auto const a7_0{*first};
    return {static_cast<Address::value_type>(a13_8 | a7_0),
            Address::ExtendedLoco};
  }
  // 232-252
  else if (*first <= 252u)
    return {*first, Address::Reserved};
  // 253
  else if (*first == 253u) return {*first, Address::DataTransfer};
  // 254
  else if (*first == 254u) return {*first, Address::AutomaticLogon};
  // 255
  else return {*first, Address::Idle};
}
#pragma GCC diagnostic pop

/// Decode address
///
/// \param  bytes Raw bytes
/// \return Address
constexpr Address decode_address(std::span<uint8_t const> bytes) {
  return decode_address(cbegin(bytes));
}

/// Decode address
///
/// \param  packet  Packet
/// \return Address
constexpr Address decode_address(Packet const& packet) {
  return decode_address(cbegin(packet));
}

/// Encode address
///
/// \tparam OutputIt  std::output_iterator<uint8_t>
/// \param  addr      Address
/// \param  first     Beginning of the range to encode to
/// \return Iterator to the element that follows the last element encoded
template<std::output_iterator<uint8_t> OutputIt>
constexpr OutputIt encode_address(Address addr, OutputIt first) {
  switch (addr.type) {
    case Address::UnknownService: break;
    case Address::Broadcast: *first++ = 0u; break;
    case Address::BasicLoco: *first++ = static_cast<uint8_t>(addr); break;
    case Address::BasicAccessory:
      *first++ = static_cast<uint8_t>(0x80u |                     //
                                      ((addr >> 2u) & 0x3Fu));    // A7-2
      *first++ = static_cast<uint8_t>(0x80u |                     //
                                      ((~addr & 0x0700u) >> 4u) | // A10-8
                                      ((addr & 0x03u) << 1u));    // A1-0
      break;
    case Address::ExtendedAccessory:
      *first++ = static_cast<uint8_t>(0x80u |                     //
                                      ((addr >> 2u) & 0x3Fu));    // A7-2
      *first++ = static_cast<uint8_t>(((~addr & 0x0700u) >> 4u) | // A10-8
                                      ((addr & 0x03u) << 1u) |    // A1-0
                                      0x01u);                     //
      break;
    case Address::ExtendedLoco:
      *first++ = static_cast<uint8_t>(0xC0u | addr >> 8u);
      *first++ = static_cast<uint8_t>(addr);
      break;
    case Address::Reserved: assert(false); break;
    case Address::DataTransfer: *first++ = 253u; break;
    case Address::AutomaticLogon: *first++ = 254u; break;
    case Address::Idle: *first++ = 255u; break;
  }
  return first;
}

} // namespace dcc
