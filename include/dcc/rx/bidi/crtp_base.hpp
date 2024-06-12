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
#include "../../bidi/kmh.hpp"
#include "../../bidi/temperature.hpp"
#include "../../bidi/track_voltage.hpp"
#include "../../zimo_id.hpp"
#include "../decoder.hpp"
#include "backoff.hpp"
#include "high_current.hpp"

namespace dcc::rx::bidi {

using namespace ::dcc::bidi;
using namespace std::chrono_literals;

/// CRTP base for transmitting BiDi
///
/// \tparam T Type to downcast to
template<typename T>
struct CrtpBase {
  friend T;

  /// Add dyn (ID7) datagrams to deque
  ///
  /// \tparam Dyns... Types of dyn datagrams
  /// \param  dyns... Datagrams
  template<std::derived_from<Dyn>... Dyns>
  void datagram(Dyns&&... dyns) {
    // Block full and release empty deque to avoid getting the same datagrams
    // send over and over again...
    if (full(_dyn_deque)) _block_dyn_deque = true;
    else if (empty(_dyn_deque)) {
      _block_dyn_deque = false;
      dyn(_qos, 7u);
    }

    // Only allow pushing datagrams if not blocked
    if (!_block_dyn_deque) (dyn(std::forward<Dyns>(dyns)), ...);
  }

  /// Start channel1 (12 bit payload)
  void cutoutChannel1() {
    // Only send in channel1 if last valid address was broadcast, short or long
    if (_addrs.received.type == Address::Broadcast ||
        _addrs.received.type == Address::Short ||
        _addrs.received.type == Address::Long)
      appAdr();
    // or automatic logon
    else if (_addrs.received.type == Address::AutomaticLogon) appLogon(1u);
  }

  /// Start channel2 (36 bit payload)
  void cutoutChannel2() {
    // Only send in channel2 if last valid address was own
    if ((_addrs.received == _addrs.primary && !_logon_assigned) ||
        (_addrs.received == _addrs.logon && _logon_assigned))
      empty(_pom_deque) ? appDyn() : appPom();
    // or consist
    else if (_addrs.received == _addrs.consist && !_logon_assigned &&
             _ch2_consist_enabled)
      appDyn();
    // or automatic logon
    else if (_addrs.received.type == Address::AutomaticLogon) appLogon(2u);
    // or broadcast
    else if (_addrs.received.type == Address::Broadcast) appTos();
  }

protected:
  constexpr CrtpBase() = default;
  Decoder auto& impl() { return static_cast<T&>(*this); }
  Decoder auto const& impl() const { return static_cast<T const&>(*this); }

  /// Execute adds adr (ID1/2) and dyn (ID7) datagrams to deque in thread mode
  void executeThreadMode() {
    adr();
    logonStore();
    updateTimepoints();
  }

  /// Configure
  ///
  /// \param  enabled             BiDi enabled
  /// \param  ch2_consist_enabled CH2 is used by consist address as well
  void config(bool enabled, bool ch2_consist_enabled) {
    auto const cv28{impl().readCv(28u - 1u)};
    _ch1_addr_enabled = enabled && (cv28 & ztl::make_mask(0u));
    _ch2_enabled = enabled && (cv28 & ztl::make_mask(1u));
    _logon_enabled = enabled && (cv28 & ztl::make_mask(7u));
    _ch2_consist_enabled = enabled && ch2_consist_enabled;
    if constexpr (HighCurrent<T>)
      impl().highCurrentBiDi(cv28 & ztl::make_mask(6u));
    _did = {impl().readCv(250u - 1u),
            impl().readCv(251u - 1u),
            impl().readCv(252u - 1u),
            impl().readCv(253u - 1u)};
    _cids.front() = static_cast<decltype(_cids)::value_type>(
      (impl().readCv(65297u - 1u) << 8u) | impl().readCv(65298u - 1u));
    _session_ids.front() = impl().readCv(65299u - 1u);
  }

  /// Quality of service
  ///
  /// \param  qos Quality of service
  void qos(uint8_t qos) { _qos = qos; }

  /// Add to pom deque
  ///
  /// \param  value CV value
  void pom(uint8_t value) {
    if (!_ch2_enabled || full(_pom_deque)) return;
    _pom_deque.push_back(encode_datagram(make_datagram<Bits::_12>(0u, value)));
  }

  /// Tip-off search
  void tipOffSearch() {
    if (_tos_backoff || !empty(_tos_deque)) return;
    auto const sec{std::chrono::duration_cast<std::chrono::seconds>(
      _last_packet_tp - _tos_tp)};
    if (sec >= 30s) return;
    auto& packet{*end(_tos_deque)};
    auto const adr_high{adrHigh(_addrs.primary)};
    auto it{std::copy(cbegin(adr_high), cend(adr_high), begin(packet))};
    auto const adr_low{adrLow(_addrs.primary)};
    it = std::copy(cbegin(adr_low), cend(adr_low), it);
    auto const time{encode_datagram(
      make_datagram<Bits::_12>(14u, static_cast<uint32_t>(sec.count())))};
    std::copy(cbegin(time), cend(time), it);
    _tos_deque.push_back();
  }

  /// Logon enable
  ///
  /// \param  gg          Address group
  /// \param  cid         Command station ID
  /// \param  session_id  Session ID
  void logonEnable(AddressGroup gg, uint16_t cid, uint8_t session_id) {
    if (!_logon_enabled) return;

    // Store new CID and session ID
    _cids.back() = cid;
    _session_ids.back() = session_id;

    // Exceptions
    if (_cids.back() == _cids.front()) {
      // Difference decides whether fast logon or skip
      auto const session_id_diff{
        static_cast<uint32_t>(_session_ids.back() - _session_ids.front())};
      auto const skip{session_id_diff <= 4u};
      _logon_selected = _logon_assigned = _logon_store = skip;

      // Skip logon if diff between session IDs is <=4
      if (skip) {
        std::array const cv65300_65301{impl().readCv(65300u - 1u),
                                       impl().readCv(65301u - 1u)};
        _addrs.logon = decode_address(cbegin(cv65300_65301));
        return;
      }
      // Force new logon if diff is >4
      else
        _addrs.logon = 0u;
    }

    switch (gg) {
      case AddressGroup::All: [[fallthrough]];  // All decoders
      case AddressGroup::Loco: break;           // Multi-function decoders
      case AddressGroup::Acc: return;           // Accessory decoder
      case AddressGroup::Now: _logon_backoff.now(); break;  // No backoff
    }

    if (_logon_backoff) return;
    assert(!full(_logon_deque));
    _logon_deque.push_back(encode_datagram(
      make_datagram<Bits::_48>(15u,
                               static_cast<uint64_t>(zimo_id) << 32u |
                                 static_cast<uint32_t>(_did[0uz]) << 24u |
                                 static_cast<uint32_t>(_did[1uz]) << 16u |
                                 static_cast<uint32_t>(_did[2uz]) << 8u |
                                 static_cast<uint32_t>(_did[3uz]))));
  }

  /// Logon select
  ///
  /// \param  did Unique ID
  void logonSelect(std::span<uint8_t const, 4uz> did) {
    if (!_logon_enabled || _logon_assigned || !std::ranges::equal(did, _did))
      return;
    _logon_selected = true;
    std::array<uint8_t, 5uz> data{
      static_cast<uint8_t>(ztl::make_mask(7u) | (_addrs.primary >> 8u)),
      static_cast<uint8_t>(_addrs.primary),
      0u,
      0u,
      0u};
    assert(!full(_logon_deque));
    _logon_deque.push_back(encode_datagram(make_datagram<Bits::_48>(
      static_cast<uint64_t>(data[0uz]) << 40u |
      static_cast<uint64_t>(data[1uz]) << 32u |
      static_cast<uint32_t>(data[2uz]) << 24u |
      static_cast<uint32_t>(data[3uz]) << 16u |
      static_cast<uint32_t>(data[4uz]) << 8u | crc8(data))));
  }

  /// Logon assign
  ///
  /// \param  did                       Unique ID
  /// \param  addr                      Assigned address
  /// \param  overwrite_primary_address Overwrite primary address
  void logonAssign(std::span<uint8_t const, 4uz> did,
                   Address addr,
                   bool overwrite_primary_address) {
    if (!_logon_enabled || !std::ranges::equal(did, _did)) return;
    _logon_assigned = _logon_store = true;
    _addrs.logon = addr;
    if (addr && overwrite_primary_address) _addrs.primary = addr;  // Stupid
    static constexpr std::array<uint8_t, 5uz> data{
      13u << 4u | 0u, 0u, 0u, 0u, 0u};
    assert(!full(_logon_deque));
    _logon_deque.push_back(encode_datagram(make_datagram<Bits::_48>(
      static_cast<uint64_t>(data[0uz]) << 40uz |
      static_cast<uint64_t>(data[1uz]) << 32uz | data[2uz] << 24uz |
      data[3uz] << 16uz | data[4uz] << 8uz | crc8(data))));
  }

  // Shared with derived class
  Addresses _addrs{};

private:
  /// Addr adr datagrams
  void adr() {
    if (!_ch1_addr_enabled || !empty(_adr_deque)) return;
    // Active address is logon
    else if (_addrs.logon) {
      _adr_deque.push_back(adrHigh(_addrs.logon));
      _adr_deque.push_back(adrLow(_addrs.logon));
    }
    // Active address is primary
    else if (!_addrs.consist) {
      _adr_deque.push_back(adrHigh(_addrs.primary));
      _adr_deque.push_back(adrLow(_addrs.primary));
    }
    // Active address is consist
    else if (_addrs.consist > 0u && _addrs.consist < 128u) {
      _adr_deque.push_back(adrHigh(_addrs.consist));
      _adr_deque.push_back(adrLow(_addrs.consist));
    }
    // RCN-217 can't encode CV20 yet
    else {
      // TODO
    }
  }

  /// Add generic dyn datagram
  ///
  /// \param  d Generic dyn datagram
  void dyn(Dyn d) { dyn(d.d, d.x); }

  /// Add kmh dyn datagram
  ///
  /// \param  kmh Kmh dyn datagram
  void dyn(Kmh kmh) {
    auto const tmp{kmh < 512 ? (kmh < 256 ? kmh : kmh - 256) : 255};
    dyn(static_cast<uint8_t>(tmp), kmh < 256 ? 0u : 1u);
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
  void dyn(DirectionStatusByte dir_stat) {
    dyn(static_cast<uint8_t>(dir_stat), 27u);
  }

  /// Add track voltage dyn datagram
  ///
  /// \param  track_voltage Track voltage dyn datagram
  void dyn(TrackVoltage track_voltage) {
    auto const track_voltage_with_dc_component{
      std::max<Dyn::value_type>(0, track_voltage - 5000)};
    dyn(static_cast<uint8_t>(track_voltage_with_dc_component / 100), 46u);
  }

  /// Add app:dyn datagrams
  ///
  /// \param  d DV (dynamic CV)
  /// \param  x Subindex
  void dyn(uint8_t d, uint8_t x) {
    if (!_ch2_enabled || full(_dyn_deque)) return;
    _dyn_deque.push_back(encode_datagram(
      make_datagram<Bits::_18>(7u, static_cast<uint32_t>(d << 6u | x))));
  }

  /// Handle app:adr_low and app:adr_high datagrams
  void appAdr() {
    if (empty(_adr_deque)) return;
    _ch1 = _adr_deque.front();
    impl().transmitBiDi({cbegin(_ch1), size(_ch1)});
    _adr_deque.pop_front();
  }

  /// Handle app::pom
  void appPom() {
    if (empty(_pom_deque)) return;
    auto const& packet{_pom_deque.front()};
    std::copy(cbegin(packet), cend(packet), begin(_ch2));
    impl().transmitBiDi({cbegin(_ch2), size(packet)});
    _pom_deque.pop_front();
  }

  /// Handle app:dyn datagrams
  void appDyn() {
    if (empty(_dyn_deque)) return;
    auto first{begin(_ch2)};
    auto const last{cend(_ch2)};
    do {
      auto const& packet{_dyn_deque.front()};
      first = std::copy_n(cbegin(packet), size(packet), first);
      _dyn_deque.pop_front();
    } while (!empty(_dyn_deque) && last - first >= ssize(_dyn_deque.front()));
    impl().transmitBiDi({cbegin(_ch2), first});
  }

  /// Handle app:tos
  void appTos() {
    if (empty(_tos_deque)) return;
    auto const& packet{_tos_deque.front()};
    std::ranges::copy(packet, begin(_ch2));
    impl().transmitBiDi({cbegin(_ch2), size(packet)});
    _tos_deque.pop_front();
  }

  /// Handle app::logon
  void appLogon(uint32_t ch) {
    if (empty(_logon_deque)) return;
    if (auto const& packet{_logon_deque.front()}; ch == 1u) {
      std::copy(begin(packet), begin(packet) + 2, begin(_ch1));
      impl().transmitBiDi({cbegin(_ch1), size(_ch1)});
    } else {
      std::copy(begin(packet) + 2, end(packet), begin(_ch2));
      impl().transmitBiDi({cbegin(_ch2), size(_ch2)});
      _logon_deque.pop_front();
    }
  }

  /// Get app:adr_high
  ///
  /// \param  addr  Address
  /// \return Datagram for app:adr_high
  auto adrHigh(Address addr) const {
    return encode_datagram(make_datagram<Bits::_12>(
      1u,
      addr == _addrs.consist
        ? 0b0110'0000u
        : (addr < 128u ? 0u : 0x80u | (addr & 0x3F00u) >> 8u)));
  }

  /// Get app:adr_low
  ///
  /// \param  addr  Address
  /// \return Datagram for app:adr_low
  auto adrLow(Address addr) const {
    return encode_datagram(make_datagram<Bits::_12>(
      2u,
      addr == _addrs.consist
        ? (_addrs.consist.reversed << 7u | (_addrs.consist & 0x007Fu))
        : (addr & 0x00FFu)));
  }

  /// Logon store
  ///
  /// RCN218 requires us to answer extended packets directly in the following
  /// cutout. This is so time-critical that logon information can only be stored
  /// asynchronously...
  void logonStore() {
    if (!_logon_store) return;
    _logon_store = false;

    // Doesn't conform to standard...
    if (_addrs.primary == _addrs.logon) {
      impl().writeCv(
        17u - 1u, static_cast<uint8_t>(0b1100'0000u | (_addrs.primary >> 8u)));
      impl().writeCv(18u - 1u, static_cast<uint8_t>(_addrs.primary));
      impl().writeCv(29u - 1u, true, 5u);
    }

    _cids.front() = _cids.back();
    impl().writeCv(65297u - 1u, static_cast<uint8_t>(_cids.back() >> 8u));
    impl().writeCv(65298u - 1u, static_cast<uint8_t>(_cids.back()));

    _session_ids.front() = _session_ids.back();
    impl().writeCv(65299u - 1u, _session_ids.back());
    impl().writeCv(65300u - 1u, static_cast<uint8_t>(_addrs.logon >> 8u));
    impl().writeCv(65301u - 1u, static_cast<uint8_t>(_addrs.logon));
  }

  /// Update time points
  ///
  /// In case time between two packets is >=2s allow tip-off search again.
  void updateTimepoints() {
    auto const packet_tp{std::chrono::system_clock::now()};
    if (packet_tp - _last_packet_tp >= 2s) {
      _tos_backoff.now();
      _tos_tp = packet_tp;
    }
    _last_packet_tp = packet_tp;
  }

  // Timepoints
  std::chrono::time_point<std::chrono::system_clock> _last_packet_tp{};
  std::chrono::time_point<std::chrono::system_clock> _tos_tp{};

  std::array<uint8_t, 4uz> _did{};

  // Buffers
  Channel1 _ch1{};
  Channel2 _ch2{};

  // Deques
  ztl::inplace_deque<std::array<uint8_t, datagram_size<Bits::_18>>,
                     DCC_RX_BIDI_DEQUE_SIZE>
    _dyn_deque{};
  ztl::inplace_deque<Channel1, DCC_RX_BIDI_DEQUE_SIZE> _pom_deque{};
  ztl::inplace_deque<Channel1, 2uz> _adr_deque{};
  ztl::inplace_deque<Channel2, 2uz> _tos_deque{};
  ztl::inplace_deque<BundledChannels, 2uz> _logon_deque{};

  Backoff _logon_backoff{};
  Backoff _tos_backoff{};

  std::array<uint16_t, 2uz> _cids{};        ///< Central ID
  std::array<uint8_t, 2uz> _session_ids{};  ///< Session ID
  uint8_t _qos{};                           ///< Quality of service

  // Not bitfields as those are most likely mutated in interrupt context
  bool _ch2_consist_enabled{};
  bool _logon_enabled{};
  bool _logon_selected{};
  bool _logon_assigned{};
  bool _logon_store{};

  bool _ch1_addr_enabled : 1 {};
  bool _ch2_enabled : 1 {};
  bool _block_dyn_deque : 1 {};
};

}  // namespace dcc::rx::bidi