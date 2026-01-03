// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi utilities
///
/// \file   dcc/bidi/acks.hpp
/// \author Vincent Hamp
/// \date   21/12/2025

#pragma once

#include <cassert>
#include <cstdint>
#include "../address.hpp"
#include "app/adr_high.hpp"
#include "app/adr_low.hpp"
#include "app/block.hpp"
#include "app/cv_auto.hpp"
#include "app/dyn.hpp"
#include "app/error.hpp"
#include "app/ext.hpp"
#include "app/info.hpp"
#include "app/info1.hpp"
#include "app/pom.hpp"
#include "app/search.hpp"
#include "app/srq.hpp"
#include "app/stat1.hpp"
#include "app/stat4.hpp"
#include "app/test.hpp"
#include "app/time.hpp"
#include "app/xpom.hpp"
#include "datagram.hpp"

namespace dcc::bidi {

/// Make app:pom datagram
///
/// \return app:pom datagram
constexpr auto make_app_pom_datagram(uint8_t byte) {
  return encode_datagram(make_datagram<Bits::_12>(app::Pom::id, byte));
}

/// Make app:adr_high datagram
///
/// \param  addr  Address
/// \param  cv19  CV19
/// \return app:adr_high datagram
constexpr auto make_app_adr_high_datagram(Address::value_type addr,
                                          uint8_t cv19 = 0u) {
  if (addr < 128u)
    return encode_datagram(make_datagram<Bits::_12>(
      app::AdrHigh::id, cv19 ? 0b0110'0000u : 0b0000'0000u));
  else
    return encode_datagram(make_datagram<Bits::_12>(
      app::AdrHigh::id, 0x80u | (addr & 0x3F00u) >> 8u));
}

/// Make app:adr_low datagram
///
/// \param  addr  Address
/// \param  cv19  CV19
/// \return app:adr_low datagram
constexpr auto make_app_adr_low_datagram(Address::value_type addr,
                                         uint8_t cv19 = 0u) {
  if (addr < 128u)
    return encode_datagram(make_datagram<Bits::_12>(
      app::AdrLow::id, (cv19 & 0x80) | (addr & 0x7Fu)));
  else
    return encode_datagram(
      make_datagram<Bits::_12>(app::AdrLow::id, addr & 0xFFu));
}

/// Make app:info1 datagram
///
/// \param  info1 app::Info1
/// \return app:info1 datagram
constexpr auto make_app_info1_datagram(app::Info1 info1) {
  return encode_datagram(
    make_datagram<Bits::_12>(app::Info1::id, std::to_underlying(info1.d)));
}

/// Make app:ext datagram
///
/// \param  ext app::Ext
/// \return app:ext datagram
constexpr auto make_app_ext_datagram(app::Ext ext) {
  assert((ext.t >= app::Ext::Reserved8 && ext.p <= 0xFFu) ||
         (ext.t < app::Ext::Reserved8 && ext.p <= 0x7FFu));
  return encode_datagram(make_datagram<Bits::_18>(
    app::Ext::id,
    static_cast<uint32_t>(std::to_underlying(ext.t) << 8u) | ext.p));
}

/// \todo not yet specified
constexpr auto make_app_info_datagram() {}

/// Make app:dyn datagram
///
/// \return app:dyn datagram
constexpr auto make_app_dyn_datagram(uint8_t d, uint8_t x) {
  assert(x < smath::pow(2u, 6u));
  return encode_datagram(
    make_datagram<Bits::_18>(app::Dyn::id, static_cast<uint32_t>(d << 6u | x)));
}

/// Make app:xpom datagram
///
/// \return app:xpom datagram
constexpr auto make_app_xpom_datagram(uint8_t ss,
                                      std::span<uint8_t const, 4uz> bytes) {
  assert(ss < size(app::Xpom::ids));
  return encode_datagram(make_datagram<Bits::_36>(
    app::Xpom::ids[ss],
    static_cast<uint32_t>(bytes[0uz] << 24u | bytes[1uz] << 16u |
                          bytes[2uz] << 8u | bytes[3uz] << 0u)));
}

/// Make app:CV-auto datagram
///
/// \return app:CV-auto datagram
constexpr auto make_app_cv_auto_datagram(uint32_t cv_addr, uint8_t byte) {
  assert(cv_addr < smath::pow(2u, 24u));
  return encode_datagram(make_datagram<Bits::_36>(
    app::CvAuto::id, cv_addr << 8u | static_cast<uint32_t>(byte << 0u)));
}

/// \todo not yet specified
constexpr auto make_app_block_datagram() {}

/// Make app:search datagram
///
/// \param  addr  Address
/// \param  cv19  CV19
/// \param  s     Time since rerailing in seconds
/// \return app:search datagram
constexpr auto
make_app_search_datagram(Address::value_type addr, uint8_t cv19, uint8_t s) {
  Datagram<datagram_size<Bits::_36>> datagram{};
  auto const adr_high{make_app_adr_high_datagram(addr, cv19)};
  auto it{std::copy(cbegin(adr_high), cend(adr_high), begin(datagram))};
  auto const adr_low{make_app_adr_low_datagram(addr, cv19)};
  it = std::copy(cbegin(adr_low), cend(adr_low), it);
  auto const time{
    encode_datagram(make_datagram<Bits::_12>(app::Search::id, s))};
  std::copy(cbegin(time), cend(time), it);
  return datagram;
}

/// Make app:srq datagram
///
/// \param  addr  Address
/// \return app:srq datagram
constexpr auto make_app_srq_datagram(Address addr) {
  assert(addr.type == Address::BasicAccessory ||
         addr.type == Address::ExtendedAccessory);
  return encode_datagram(make_datagram<Bits::_12>(
    static_cast<uint8_t>(
      (addr.type == Address::BasicAccessory ? 0b0000u : 0b1000u) |
      (addr >> 8u)),
    static_cast<uint8_t>(addr)));
}

/// Make app:stat4 datagram
///
/// \param  cv33  CV33
/// \return app:stat4 datagram
constexpr auto make_app_stat4_datagram(uint8_t cv33) {
  return encode_datagram(make_datagram<Bits::_12>(app::Stat4::id, cv33));
}

/// Make app:stat1 datagram
///
/// \param  d Data
/// \return app:stat1 datagram
constexpr auto make_app_stat1_datagram(uint8_t d) {
  return encode_datagram(make_datagram<Bits::_12>(app::Stat1::id, d));
}

/// Make app:time datagram
///
/// \param  res   Resolution (0.1s or 1s)
/// \param  time  Time
/// \return app:time datagram
constexpr auto make_app_time_datagram(bool res, uint8_t time) {
  return encode_datagram(make_datagram<Bits::_12>(
    app::Time::id, static_cast<uint8_t>(res << 7u | time)));
}

/// Make app:error datagram
///
/// \param  code  Error code
/// \return app:error datagram
constexpr auto make_app_error_datagram(app::Error::Code code) {
  return encode_datagram(
    make_datagram<Bits::_12>(app::Error::id, std::to_underlying(code)));
}

} // namespace dcc::bidi
