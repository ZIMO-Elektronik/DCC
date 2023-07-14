// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive BiDi base
///
/// \file   dcc/rx/bidi/crtp_base.hpp
/// \author Vincent Hamp
/// \date   04/01/2022

#pragma once

#include <chrono>
#include <concepts>
#include <ztl/inplace_deque.hpp>
#include "../../addresses.hpp"
#include "../../bidi/bundled_channels.hpp"
#include "../../bidi/datagram.hpp"
#include "../../bidi/direction_status_byte.hpp"
#include "../../bidi/dyn.hpp"
#include "../../bidi/speed.hpp"
#include "../../bidi/temperature.hpp"
#include "../../bidi/track_voltage.hpp"
#include "../../zimo_id.hpp"
#include "../decoder.hpp"
#include "high_current.hpp"
#include "logon_backoff.hpp"
#include "packet.hpp"

namespace dcc::rx::bidi {

using namespace std::chrono_literals;

/// CRTP base for transmitting BiDi
///
/// \tparam T Type to downcast to
template<typename T>
struct CrtpBase {
  friend T;

  /// Initialize
  void init() { _logon_backoff.reset(); }

protected:
  constexpr CrtpBase() = default;
  Decoder auto& impl() { return static_cast<T&>(*this); }
  Decoder auto const& impl() const { return static_cast<T const&>(*this); }

  /// Execute adds dyn (ID7) datagrams to queue
  ///
  /// \tparam Dyns... Types of dyn datagrams
  /// \param  dyns... Messages
  template<std::derived_from<Dyn>... Dyns>
  void execute(Dyns&&... dyns)
    requires((sizeof...(Dyns) <
              DCC_RX_BIDI_QUEUE_SIZE))  // TODO remove double braces, currently
                                        // fucks with VSCode highlighting
  {
    if (!sizeof...(Dyns) ||
        (DCC_RX_BIDI_QUEUE_SIZE - size(_dyn_queue) < sizeof...(Dyns) + 1uz))
      return;
    (dyn(std::forward<Dyns>(dyns)), ...);
    dyn(_qos, 7u);
  }

  /// Configure
  ///
  /// \param  enabled             BiDi enabled
  /// \param  ch2_consist_enabled CH2 is used by consist address as well
  void config(bool enabled, bool ch2_consist_enabled) {
    auto const cv28{impl().readCv(28u - 1u)};
    _ch1_enabled = enabled && (cv28 & ztl::make_mask(0u));
    _ch2_enabled = enabled && (cv28 & ztl::make_mask(1u));
    _ch2_consist_enabled = ch2_consist_enabled;
    if constexpr (HighCurrent<T>) impl().highCurrent(cv28 & ztl::make_mask(6u));
    _did = {impl().readCv(250u - 1u),
            impl().readCv(251u - 1u),
            impl().readCv(252u - 1u),
            impl().readCv(253u - 1u)};
    std::array const cv65297_65298{impl().readCv(65297u - 1u),
                                   impl().readCv(65298u - 1u)};
    _addrs.logon = decode_address(begin(cv65297_65298));
    _cid = static_cast<decltype(_cid)>((impl().readCv(65299u - 1u) << 8u) |
                                       impl().readCv(65300u - 1u));
    _session_id = impl().readCv(65301u - 1u);
  }

  /// Execute in thread mode
  void executeThreadMode() {
    logonStore();
    updateTimepoints();
  }

  /// Start channel1 (12 bit payload)
  void cutoutChannel1() {
    if (!_ch1_enabled) return;
    // Only send in channel1 if last valid address was broadcast, short or long
    if (_addrs.received.type == Address::Broadcast ||
        _addrs.received.type == Address::Short ||
        _addrs.received.type == Address::Long)
      appAdr();
    // or extended packet
    else if (_addrs.received.type == Address::ExtendedPacket) appLogon(1u);
  }

  /// Start channel2 (36 bit payload)
  void cutoutChannel2() {
    if (!_ch2_enabled) return;
    // Only send in channel2 if last valid address was own
    if (_addrs.received == _addrs.primary ||
        (_logon_assigned && _addrs.received == _addrs.logon))
      appPomExtDynSubId();
    // or consist
    else if (_ch2_consist_enabled && _addrs.received == _addrs.consist)
      appExtDynSubId();
    // or tip-off search
    else if (_addrs.received.type == Address::TipOffSearch) appTos();
    // or extended packet
    else if (_addrs.received.type == Address::ExtendedPacket) appLogon(2u);
  }

  /// Quality of service
  ///
  /// \param  qos Quality of service
  void qos(uint8_t qos) { _qos = qos; }

  /// Add to pom queue
  ///
  /// \param  value CV value
  void pom(uint8_t value) {
    if (_pom_queue.full()) return;
    _pom_queue.push_back(encode_datagram(make_datagram<Bits::_12>(0u, value)));
  }

  /// Tip-off search
  void tipOffSearch() {
    if (constexpr auto six_pct{static_cast<decltype(rand())>(RAND_MAX * 0.06)};
        !empty(_tos_queue) || rand() > six_pct)
      return;
    auto const sec{std::chrono::duration_cast<std::chrono::seconds>(
      _last_packet_tp - _tos_tp)};
    if (sec >= 30s) return;
    auto& packet{*end(_tos_queue)};
    auto const adr_high{adrHigh()};
    auto it{std::copy(cbegin(adr_high), cend(adr_high), begin(packet))};
    auto const adr_low{adrLow()};
    it = std::copy(cbegin(adr_low), cend(adr_low), it);
    auto const time{encode_datagram(
      make_datagram<Bits::_12>(0u, static_cast<uint32_t>(sec.count())))};
    std::copy(cbegin(time), cend(time), it);
    _tos_queue.push_back();
  }

  /// Logon enable
  ///
  /// \param  gg          Address group
  /// \param  cid         Command station ID
  /// \param  session_id  Session ID
  void logonEnable(uint8_t gg, uint16_t cid, uint8_t session_id) {
    auto const _cidequal{cid == _cid};
    auto const session_id_equal{session_id == _session_id};

    // Exceptions
    // - Either skip logon if diff between session IDs is <=4
    // - Or force new logon if diff is >4
    if (_cidequal && !session_id_equal) {
      _logon_selected = _logon_assigned = session_id - _session_id <= 4u;
      _logon_backoff.reset();
    }

    _cid = cid;
    _session_id = session_id;

    if (_logon_selected || _logon_assigned) return;
    switch (gg) {
      case 0b00u: [[fallthrough]];              // All decoders
      case 0b01u: break;                        // Multi-function decoders
      case 0b10u: return;                       // Accessory decoder
      case 0b11u: _logon_backoff.now(); break;  // No backoff
    }

    if (_logon_backoff) return;
    _logon_queue.push_back(encode_datagram(make_datagram<Bits::_48>(
      15u,
      static_cast<uint64_t>(zimo_id) << 32u | _did[0uz] << 24u |
        _did[1uz] << 16u | _did[2uz] << 8u | _did[3uz])));
  }

  /// Logon select
  ///
  /// \param  did Unique ID
  void logonSelect(std::span<uint8_t const, 4uz> did) {
    if (_logon_assigned || !std::ranges::equal(did, _did)) return;
    _logon_selected = true;
    std::array<uint8_t, 5uz> data{
      static_cast<uint8_t>(ztl::make_mask(7u) | (_addrs.primary >> 8u)),
      static_cast<uint8_t>(_addrs.primary),
      0u,
      0u,
      0u};
    _logon_queue.push_back(encode_datagram(make_datagram<Bits::_48>(
      static_cast<uint64_t>(data[0uz]) << 40uz |
      static_cast<uint64_t>(data[1uz]) << 32uz | data[2uz] << 24uz |
      data[3uz] << 16uz | data[4uz] << 8uz | crc8(data))));
  }

  /// Logon assign
  ///
  /// \param  did   Unique ID
  /// \param  addr  Assigned address
  void logonAssign(std::span<uint8_t const, 4uz> did, Address addr) {
    if (!std::ranges::equal(did, _did)) return;
    _logon_assigned = _logon_store = true;
    // Fucking stupid and doesn't conform to standard...
    _addrs.primary = _addrs.logon = addr;
    static constexpr std::array<uint8_t, 5uz> data{
      13u << 4u | 0u, 0u, 0u, 0u, 0u};
    _logon_queue.push_back(encode_datagram(make_datagram<Bits::_48>(
      static_cast<uint64_t>(data[0uz]) << 40uz |
      static_cast<uint64_t>(data[1uz]) << 32uz | data[2uz] << 24uz |
      data[3uz] << 16uz | data[4uz] << 8uz | crc8(data))));
  }

  // Shared with derived class
  Addresses _addrs{};

private:
  /// Add generic dyn datagram
  ///
  /// \param  d Generic dyn datagram
  void dyn(Dyn d) { dyn(d.d, d.x); }

  /// Add speed dyn datagram
  ///
  /// \param  speed Speed dyn datagram
  void dyn(Speed speed) {
    auto const tmp{speed < 512 ? (speed < 256 ? speed : speed - 256) : 255};
    dyn(static_cast<uint8_t>(tmp), speed < 256 ? 0u : 1u);
  }

  /// Add temperature dyn datagram
  ///
  /// \param  temp  Temperature dyn datagram
  void dyn(Temperature temp) {
    auto const tmp{ztl::lerp<Dyn::value_type>(temp.d, -50, 205, 0, 255)};
    dyn(static_cast<uint8_t>(tmp), 26u);
  }

  /// Add direction status dyn datagram
  ///
  /// \param  dir_stat  Direction status byte dyn datagram
  void dyn(DirectionStatusByte dir_stat) { dyn(dir_stat, 27u); }

  /// Add track voltage dyn datagram
  ///
  /// \param  track_voltage Track voltage dyn datagram
  void dyn(TrackVoltage track_voltage) {
    dyn(static_cast<uint8_t>(track_voltage / 100), 46u);
  }

  /// Handle app:dyn datagrams
  ///
  /// \param  d DV (dynamic CV)
  /// \param  x Subindex
  void dyn(uint8_t d, uint8_t x) {
    auto const dyn{encode_datagram(
      make_datagram<Bits::_18>(7u, static_cast<uint32_t>(d << 6u | x)))};
    std::copy(begin(dyn), end(dyn), begin(end(_dyn_queue)->data));
    end(_dyn_queue)->size = size(dyn);
    _dyn_queue.push_back();
  }

  /// Handle app:adr_low and app:adr_high datagrams
  void appAdr() {
    // TODO consist currently never sent!
    auto const adr_high{adrHigh()};
    _ch1 = _ch1 == adr_high ? adrLow() : adr_high;
    impl().transmitBiDi({cbegin(_ch1), size(_ch1)});
  }

  /// Handle app:pom, app:ext, app:dyn and app:subID datagrams
  void appPomExtDynSubId() {
    // Send either pom only (no fucking thanks ESU)
    if (!empty(_pom_queue)) appPom();
    // Or whatever fits into channel2
    else appExtDynSubId();
  }

  /// Handle app::pom
  void appPom() {
    auto const& packet{_pom_queue.front()};
    std::copy(cbegin(packet), cend(packet), begin(_ch2));
    impl().transmitBiDi({cbegin(_ch2), size(packet)});
    _pom_queue.pop_front();
  }

  /// Handle app:ext, app:dyn and app:subID datagrams
  void appExtDynSubId() {
    if (empty(_dyn_queue)) return;
    auto first{begin(_ch2)};
    auto const last{cend(_ch2)};
    do {
      auto const& packet{_dyn_queue.front()};
      first = std::copy_n(cbegin(packet.data), packet.size, first);
      _dyn_queue.pop_front();
    } while (!empty(_dyn_queue) && last - first >= _dyn_queue.front().size);
    impl().transmitBiDi({cbegin(_ch2), first});
  }

  /// Handle app:tos
  void appTos() {
    if (empty(_tos_queue)) return;
    auto const& packet{_tos_queue.front()};
    std::ranges::copy(packet, begin(_ch2));
    impl().transmitBiDi({begin(_ch2), size(packet)});
    _tos_queue.pop_front();
  }

  /// Handle app::logon
  void appLogon(uint32_t ch) {
    if (empty(_logon_queue)) return;
    if (auto const& packet{_logon_queue.front()}; ch == 1u) {
      std::copy(begin(packet), begin(packet) + 2, begin(_ch1));
      impl().transmitBiDi({cbegin(_ch1), size(_ch1)});
    } else {
      std::copy(begin(packet) + 2, end(packet), begin(_ch2));
      impl().transmitBiDi({cbegin(_ch2), size(_ch2)});
      _logon_queue.pop_front();
    }
  }

  /// Get app:adr_high
  ///
  /// \return Datagram for app:adr_high
  auto adrHigh() const {
    // TODO consist currently never sent!
    return encode_datagram(make_datagram<Bits::_12>(
      1u,
      _addrs.primary < 128u ? 0u : 0x80u | (_addrs.primary & 0x3F00u) >> 8u));
  }

  /// Get app:adr_low
  ///
  /// \return Datagram for app:adr_low
  auto adrLow() const {
    // TODO consist currently never sent!
    return encode_datagram(
      make_datagram<Bits::_12>(2u, _addrs.primary & 0x00FFu));
  }

  /// Logon store
  ///
  /// I fucking hate this. RCN218 stupidly requires us to answer extended
  /// packets directly in the following cutout. This is so time-critical that
  /// logon information can only be stored asynchronously...
  void logonStore() {
    if (!_logon_store) return;
    _logon_store = false;
    // This is wrong on so many levels... I can't even... FML
    impl().writeCv(17u - 1u,
                   static_cast<uint8_t>(0b1100'0000u | (_addrs.primary >> 8u)));
    impl().writeCv(18u - 1u, static_cast<uint8_t>(_addrs.primary));
    impl().writeCv(29u - 1u, true, 5u);
    impl().writeCv(65297u - 1u, static_cast<uint8_t>(_addrs.logon >> 8u));
    impl().writeCv(65298u - 1u, static_cast<uint8_t>(_addrs.logon));
    impl().writeCv(65299u - 1u, static_cast<uint8_t>(_cid >> 8u));
    impl().writeCv(65300u - 1u, static_cast<uint8_t>(_cid));
    impl().writeCv(65301u - 1u, _session_id);
  }

  /// Update time points
  ///
  /// In case time between two packets is >=2s allow tip-off search again.
  void updateTimepoints() {
    auto const packet_tp{std::chrono::system_clock::now()};
    if (packet_tp - _last_packet_tp >= 2s) _tos_tp = packet_tp;
    _last_packet_tp = packet_tp;
  }

  std::chrono::time_point<std::chrono::system_clock> _last_packet_tp{};
  std::chrono::time_point<std::chrono::system_clock> _tos_tp{};
  std::array<uint8_t, 4uz> _did{};
  Channel1 _ch1{};
  Channel2 _ch2{};
  ztl::inplace_deque<Packet, DCC_RX_BIDI_QUEUE_SIZE> _dyn_queue{};
  ztl::inplace_deque<Channel1, DCC_RX_BIDI_QUEUE_SIZE> _pom_queue{};
  ztl::inplace_deque<Channel2, 2uz> _tos_queue{};
  ztl::inplace_deque<BundledChannels, 2uz> _logon_queue{};
  LogonBackoff _logon_backoff{};
  uint16_t _cid{};        ///< Central ID
  uint8_t _session_id{};  ///< Session ID
  uint8_t _qos{};         ///< Quality of service

  // Not bitfields as those are most likely mutated in interrupt context
  bool _ch1_enabled{};
  bool _ch2_enabled{};
  bool _ch2_consist_enabled{};
  bool _logon_selected{};
  bool _logon_assigned{};
  bool _logon_store{};
};

}  // namespace dcc::rx::bidi