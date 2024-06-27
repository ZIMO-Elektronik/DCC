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
    return (lhs.type == Short || lhs.type == Long) &&
               (rhs.type == Short || rhs.type == Long)
             ? lhs.value == rhs.value
             : lhs.value == rhs.value && lhs.type == rhs.type;
  }

  constexpr operator value_type&() { return value; }
  constexpr operator value_type const&() const { return value; }

  value_type value{};

  enum : uint8_t {
    UnknownService,  ///< Address is unknown or service (=no address)
    Broadcast,       ///< Address is broadcast
    Short,           ///< Address is short
    Accessory,       ///< Address is accessory decoder
    Long,            ///< Address is long
    Reserved,        ///< Address is reserved
    DataTransfer,    ///< Address for data transfer
    AutomaticLogon,  ///< Address is automatic logon
    IdleSystem       ///< Address for system commands
  } type{};

  bool reversed{};  /// Direction reversed
};

/// Decode address
///
/// \tparam InputIt std::input_iterator
/// \param  first   Beginning of the range to decode from
/// \return Address
template<std::input_iterator InputIt>
constexpr Address decode_address(InputIt first) {
  // 0
  if (*first == 0u) return {*first, Address::Broadcast};
  // 1-127
  else if (*first <= 127u) return {*first, Address::Short};
  // 128-191
  else if (*first <= 191u)
    return {*first, Address::Accessory};  // TODO most likely wrong?
  // 192-231
  else if (*first <= 231u) {
    auto const a13_8{*first++};
    auto const a7_0{*first};
    return {static_cast<Address::value_type>(((a13_8 << 8u) | a7_0) & 0x3FFFu),
            Address::Long};
  }
  // 232-252
  else if (*first <= 252u)
    return {*first, Address::Reserved};
  // 253
  else if (*first == 253u) return {*first, Address::DataTransfer};
  // 254
  else if (*first == 254u) return {*first, Address::AutomaticLogon};
  // 255
  else return {*first, Address::IdleSystem};
}

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
    case Address::Short: *first++ = static_cast<uint8_t>(addr); break;
    case Address::Accessory:
      // TODO (see decode_address)
      break;
    case Address::Long:
      *first++ = static_cast<uint8_t>(0b1100'0000u | addr >> 8u);
      *first++ = static_cast<uint8_t>(addr);
      break;
    case Address::Reserved: assert(false); break;
    case Address::DataTransfer: *first++ = 253u; break;
    case Address::AutomaticLogon: *first++ = 254u; break;
    case Address::IdleSystem: *first++ = 255u; break;
  }
  return first;
}

}  // namespace dcc