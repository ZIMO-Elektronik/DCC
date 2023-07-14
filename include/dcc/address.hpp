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
    TipOffSearch,    ///< Address for tip-off search
    ExtendedPacket,  ///< Address is extended packet
    IdleSystem       ///< Address for system commands
  } type{};
};

/// Decode address
///
/// \param  data  Pointer to data
/// \return Address
constexpr Address decode_address(uint8_t const* data) {
  // 0
  if (data[0uz] < 1u) return {data[0uz], Address::Broadcast};
  // 1-127
  else if (data[0uz] < 128u) return {data[0uz], Address::Short};
  // 128-191
  else if (data[0uz] < 192u)
    return {data[0uz], Address::Accessory};  // TODO most likely wrong?
  // 192-231
  else if (data[0uz] < 232u) {
    auto const addr{(static_cast<uint32_t>(data[0uz] << 8u) | data[1uz]) &
                    0x3FFFu};
    return {static_cast<Address::value_type>(addr), Address::Long};
  }
  // 232-252
  else if (data[0uz] < 253u)
    return {data[0uz], Address::Reserved};
  // 253
  else if (data[0uz] < 254u) return {data[0uz], Address::TipOffSearch};
  // 254
  else if (data[0uz] < 255u) return {data[0uz], Address::ExtendedPacket};
  // 255
  else return {data[0uz], Address::IdleSystem};
}

/// Encode address
///
/// \tparam OutputIt
/// \param  addr
/// \param  data
/// \return Address
template<std::output_iterator<uint8_t> OutputIt>
constexpr OutputIt encode_address(Address addr, OutputIt data) {
  switch (addr.type) {
    case Address::UnknownService: break;
    case Address::Broadcast: *data++ = 0u; break;
    case Address::Short: *data++ = static_cast<uint8_t>(addr); break;
    case Address::Accessory:
      // TODO (see decode_address)
      break;
    case Address::Long:
      *data++ = 0b1100'0000u | addr >> 8u;
      *data++ = static_cast<uint8_t>(addr);
      break;
    case Address::Reserved: assert(false); break;
    case Address::TipOffSearch: *data++ = 253u; break;
    case Address::ExtendedPacket: *data++ = 254u; break;
    case Address::IdleSystem: *data++ = 255u; break;
  }
  return data;
}

}  // namespace dcc