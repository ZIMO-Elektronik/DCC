// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// BiDi dissector
///
/// \file   dcc/bidi/dissector.hpp
/// \author Vincent Hamp
/// \date   15/05/2025

#pragma once

#include <bit>
#include <cassert>
#include <climits>
#include <variant>
#include <ztl/limits.hpp>
#include "../address.hpp"
#include "acks.hpp"
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
#include "channel.hpp"
#include "datagram.hpp"
#include "nak.hpp"

namespace dcc::bidi {

/// Dissect datagrams
struct Dissector : std::ranges::view_interface<Dissector> {
  using value_type = std::variant<Ack,          // ACK
                                  Nak,          // NAK
                                                //
                                  app::Pom,     // ID0
                                  app::AdrHigh, // ID1
                                  app::AdrLow,  // ID2
                                  app::Info1,   // ID3
                                  app::Ext,     // ID3
                                  app::Info,    // ID4
                                  app::Dyn,     // ID7
                                  app::Xpom,    // ID8-11
                                  app::CvAuto,  // ID12
                                  app::Block,   // ID13
                                  app::Search,  // ID14
                                                //
                                  app::Srq,     //
                                                // ID0
                                  app::Stat4,   // ID3
                                  app::Stat1,   // ID4
                                  app::Time,    // ID5
                                  app::Error,   // ID6
                                                // ID7
                                                // ID8-11
                                  app::Test     // ID12
                                  >;            // ID13

  using size_type = ztl::smallest_unsigned_t<bundled_channels_size>;
  using difference_type = std::make_signed_t<size_type>;
  using reference = value_type;
  using pointer = value_type;
  using iterator_category = std::input_iterator_tag;

  constexpr Dissector() = default;
  constexpr Dissector(Datagram<> encoded, Address addr)
    : _encoded{encoded}, _addr{addr} {
    if (validate()) {
      _decoded = decode_datagram(_encoded);
      _i = static_cast<size_type>(
        (_encoded[0uz] == 0u) +
        (_encoded[1uz] == 0u)); // No channel 1 data if first two bytes are 0
    } else
      _i = std::numeric_limits<size_type>::max(); // No channel 1 and 2 data
  }
  constexpr Dissector(Datagram<> encoded, Packet packet)
    : Dissector{encoded, decode_address(packet)} {}

  constexpr Dissector& operator++() {
    auto const byte_count{std::size(next())};
    _i = byte_count > sizeof(Ack) ? static_cast<size_type>(_i + byte_count)
                                  : std::numeric_limits<size_type>::max();
    return *this;
  }

  constexpr Dissector operator++(int) {
    auto retval{*this};
    ++(*this);
    return retval;
  }

  constexpr reference operator*() const {
    auto const dg{next()};
    auto const byte_count{std::size(dg)};

    // ACK or NAK
    if (byte_count == sizeof(Ack)) {
      if (dg[0uz] == acks[0uz] || dg[0uz] == acks[1uz]) return Ack{dg[0uz]};
      else if (dg[0uz] == nak) return Nak{dg[0uz]};
    }

    auto const bit_count{byte_count * 6uz - 4uz};
    auto const id{static_cast<uint8_t>(dg[0uz] >> 2u)};
    auto const data{make_data(dg)};
    auto const d{data & ((1ull << bit_count) - 1ull)};

    switch (_addr.type) {
      case Address::Broadcast:
        switch (id) {
          case app::AdrHigh::id:
            return app::AdrHigh{.d = static_cast<uint8_t>(d)};
          case app::AdrLow::id:
            return app::AdrLow{.d = static_cast<uint8_t>(d)};
          case app::Search::id:
            return app::Search{.d = static_cast<uint8_t>(d)};
          default: return {};
        }

      case Address::BasicLoco: [[fallthrough]];
      case Address::ExtendedLoco:
        switch (id) {
          case app::Pom::id: return app::Pom{.d = static_cast<uint8_t>(d)};
          case app::AdrHigh::id:
            return app::AdrHigh{.d = static_cast<uint8_t>(d)};
          case app::AdrLow::id:
            return app::AdrLow{.d = static_cast<uint8_t>(d)};
          // case app::Info1::id: [[fallthrough]];
          case app::Ext::id:
            if (_i < 2uz)
              return app::Info1{.d = static_cast<app::Info1::Flags>(d)};
            else {
              app::Ext::Type const type{static_cast<uint8_t>(d >> 8u)};
              return app::Ext{
                .t = type >= app::Ext::Reserved8 ? type : app::Ext::AddressOnly,
                .p = static_cast<uint16_t>(
                  d & (type >= app::Ext::Reserved8 ? 0x00FFu : 0x07FFu))};
            }
          case app::Info::id: return app::Info{};
          case app::Dyn::id:
            return app::Dyn{.d = static_cast<uint8_t>(d >> 6u),
                            .x = static_cast<uint8_t>(d & 0x3Fu)};
          case app::Xpom::ids[0uz]: [[fallthrough]];
          case app::Xpom::ids[1uz]: [[fallthrough]];
          case app::Xpom::ids[2uz]: [[fallthrough]];
          case app::Xpom::ids[3uz]:
            return app::Xpom{.ss = static_cast<uint8_t>(id & 0b11u),
                             .d = {static_cast<uint8_t>(d >> 24u),
                                   static_cast<uint8_t>(d >> 16u),
                                   static_cast<uint8_t>(d >> 8u),
                                   static_cast<uint8_t>(d >> 0u)}};
          case app::CvAuto::id:
            return app::CvAuto{.v = static_cast<uint32_t>(d >> 8u),
                               .d = static_cast<uint8_t>(d)};
          case app::Block::id: return app::Block{};
          case app::Search::id:
            return app::Search{.d = static_cast<uint8_t>(d)};
          default: return {};
        }

      case Address::BasicAccessory: [[fallthrough]];
      case Address::ExtendedAccessory:
        // app:srq
        if (!_i) return app::Srq{.d = static_cast<decltype(app::Srq::d)>(data)};
        // others
        else switch (id) {
            case app::Pom::id: return app::Pom{.d = static_cast<uint8_t>(d)};
            case app::Stat4::id:
              return app::Stat4{.d = static_cast<uint8_t>(d)};
            case app::Stat1::id:
              return app::Stat1{.d = static_cast<uint8_t>(d)};
            case app::Time::id: return app::Time{.d = static_cast<uint8_t>(d)};
            case app::Error::id:
              return app::Error{.d = static_cast<app::Error::Code>(d)};
            case app::Dyn::id:
              return app::Dyn{.d = static_cast<uint8_t>(d >> 6u),
                              .x = static_cast<uint8_t>(d & 0x3Fu)};
            case app::Xpom::ids[0uz]: [[fallthrough]];
            case app::Xpom::ids[1uz]: [[fallthrough]];
            case app::Xpom::ids[2uz]: [[fallthrough]];
            case app::Xpom::ids[3uz]:
              return app::Xpom{.ss = static_cast<uint8_t>(id & 0b11u),
                               .d = {static_cast<uint8_t>(d >> 24u),
                                     static_cast<uint8_t>(d >> 16u),
                                     static_cast<uint8_t>(d >> 8u),
                                     static_cast<uint8_t>(d >> 0u)}};
            case app::Test::id: return app::Test{};
            case app::Block::id: return app::Block{};
            default: return {};
          }

      default: return {};
    }
  }

  constexpr bool operator==(std::default_sentinel_t) const {
    return _i >= bundled_channels_size || !_encoded[_i] || std::empty(next());
  }

  constexpr Dissector& begin() { return *this; }
  constexpr Dissector const& begin() const { return *this; }
  constexpr std::default_sentinel_t end() { return std::default_sentinel; }
  constexpr std::default_sentinel_t end() const {
    return std::default_sentinel;
  }
  constexpr std::default_sentinel_t cend() { return std::default_sentinel; }
  constexpr std::default_sentinel_t cend() const {
    return std::default_sentinel;
  }

private:
  constexpr bool validate() const {
    return std::ranges::all_of(_encoded, [](uint8_t b) {
      return !b || std::popcount(b) == CHAR_BIT / 2;
    });
  }

  constexpr std::span<uint8_t const> next() const {
    // No more data
    if (!_encoded[_i]) return {};

    // ACK or NAK
    if (_encoded[_i] == acks[0uz] || _encoded[_i] == acks[1uz] ||
        _encoded[_i] == nak)
      return {&_encoded[_i], sizeof(Ack)};

    switch (auto const id{static_cast<uint8_t>(_decoded[_i] >> 2u)};
            _addr.type) {
      case Address::Broadcast:
        switch (id) {
          case app::AdrHigh::id: [[fallthrough]];
          case app::AdrLow::id: [[fallthrough]];
          case app::Search::id:
            return {&_decoded[_i], datagram_size<Bits::_12>};
          default: return {};
        }

      case Address::BasicLoco: [[fallthrough]];
      case Address::ExtendedLoco:
        switch (id) {
          case app::Pom::id: [[fallthrough]];
          case app::AdrHigh::id: [[fallthrough]];
          case app::AdrLow::id:
            return {&_decoded[_i], datagram_size<Bits::_12>};
          // case app::Info1::id: [[fallthrough]];
          case app::Ext::id:
            return {&_decoded[_i],
                    _i < 2u ? datagram_size<Bits::_12>
                            : datagram_size<Bits::_18>};
          case app::Info::id: return {&_decoded[_i], datagram_size<Bits::_36>};
          case app::Dyn::id: return {&_decoded[_i], datagram_size<Bits::_18>};
          case app::Xpom::ids[0uz]: [[fallthrough]];
          case app::Xpom::ids[1uz]: [[fallthrough]];
          case app::Xpom::ids[2uz]: [[fallthrough]];
          case app::Xpom::ids[3uz]: [[fallthrough]];
          case app::CvAuto::id: [[fallthrough]];
          case app::Block::id: return {&_decoded[_i], datagram_size<Bits::_36>};
          default: return {};
        }

      case Address::BasicAccessory: [[fallthrough]];
      case Address::ExtendedAccessory:
        // app:srq
        if (!_i) return {&_decoded[_i], datagram_size<Bits::_12>};
        // others
        else switch (id) {
            case app::Pom::id: [[fallthrough]];
            case app::Stat4::id: [[fallthrough]];
            case app::Stat1::id: [[fallthrough]];
            case app::Time::id: [[fallthrough]];
            case app::Error::id:
              return {&_decoded[_i], datagram_size<Bits::_12>};
            case app::Dyn::id: return {&_decoded[_i], datagram_size<Bits::_18>};
            case app::Xpom::ids[0uz]: [[fallthrough]];
            case app::Xpom::ids[1uz]: [[fallthrough]];
            case app::Xpom::ids[2uz]: [[fallthrough]];
            case app::Xpom::ids[3uz]:
              return {&_decoded[_i], datagram_size<Bits::_36>};
            case app::Test::id: return {};
            case app::Block::id:
              return {&_decoded[_i], datagram_size<Bits::_36>};
            default: return {};
          }

      default: return {};
    }
  }

  Datagram<> _encoded{};
  Datagram<> _decoded{};
  Address _addr{};
  size_type _i{};
};

constexpr auto begin(Dissector& c) -> decltype(c.begin()) { return c.begin(); }
constexpr auto begin(Dissector const& c) -> decltype(c.begin()) {
  return c.begin();
}
constexpr auto end(Dissector& c) -> decltype(c.end()) { return c.end(); }
constexpr auto end(Dissector const& c) -> decltype(c.end()) { return c.end(); }
constexpr auto cbegin(Dissector const& c) -> decltype(c.begin()) {
  return c.begin();
}
constexpr auto cend(Dissector const& c) -> decltype(c.end()) { return c.end(); }

} // namespace dcc::bidi
