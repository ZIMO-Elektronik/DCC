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
#include <ztl/circular_array.hpp>
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
  void init() { logon_backoff_.reset(); }

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
        (DCC_RX_BIDI_QUEUE_SIZE - size(dyn_queue_) < sizeof...(Dyns) + 1uz))
      return;
    (dyn(std::forward<Dyns>(dyns)), ...);
    dyn(qos_, 7u);
  }

  /// Configure
  ///
  /// \param  enabled             BiDi enabled
  /// \param  ch2_consist_enabled CH2 is used by consist address as well
  void config(bool enabled, bool ch2_consist_enabled) {
    auto const cv28{impl().readCv(28u - 1u)};
    ch1_enabled_ = enabled && (cv28 & ztl::make_mask(0u));
    ch2_enabled_ = enabled && (cv28 & ztl::make_mask(1u));
    ch2_consist_enabled_ = ch2_consist_enabled;
    if constexpr (HighCurrent<T>) impl().highCurrent(cv28 & ztl::make_mask(6u));
    did_ = {impl().readCv(250u - 1u),
            impl().readCv(251u - 1u),
            impl().readCv(252u - 1u),
            impl().readCv(253u - 1u)};
    std::array const cv65297_65298{impl().readCv(65297u - 1u),
                                   impl().readCv(65298u - 1u)};
    addrs_.logon = decode_address(begin(cv65297_65298));
    cid_ = static_cast<decltype(cid_)>((impl().readCv(65299u - 1u) << 8u) |
                                       impl().readCv(65300u - 1u));
    session_id_ = impl().readCv(65301u - 1u);
  }

  /// Execute in thread mode
  void executeThreadMode() {
    logonStore();
    updateTimepoints();
  }

  /// Start channel1 (12 bit payload)
  void cutoutChannel1() {
    if (!ch1_enabled_) return;
    // Only send in channel1 if last valid address was broadcast, short or long
    if (addrs_.received.type == Address::Broadcast ||
        addrs_.received.type == Address::Short ||
        addrs_.received.type == Address::Long)
      appAdr();
    // or extended packet
    else if (addrs_.received.type == Address::ExtendedPacket) appLogon(1u);
  }

  /// Start channel2 (36 bit payload)
  void cutoutChannel2() {
    if (!ch2_enabled_) return;
    // Only send in channel2 if last valid address was own
    if (addrs_.received == addrs_.primary ||
        (logon_assigned_ && addrs_.received == addrs_.logon))
      appPomExtDynSubId();
    // or consist
    else if (ch2_consist_enabled_ && addrs_.received == addrs_.consist)
      appExtDynSubId();
    // or tip-off search
    else if (addrs_.received.type == Address::TipOffSearch) appTos();
    // or extended packet
    else if (addrs_.received.type == Address::ExtendedPacket) appLogon(2u);
  }

  /// Quality of service
  ///
  /// \param  qos Quality of service
  void qos(uint8_t qos) { qos_ = qos; }

  /// Add to pom queue
  ///
  /// \param  value CV value
  void pom(uint8_t value) {
    if (pom_queue_.full()) return;
    pom_queue_.push_back(encode_datagram(make_datagram<Bits::_12>(0u, value)));
  }

  /// Tip-off search
  void tipOffSearch() {
    if (constexpr auto six_pct{static_cast<decltype(rand())>(RAND_MAX * 0.06)};
        !empty(tos_queue_) || rand() > six_pct)
      return;
    auto const sec{std::chrono::duration_cast<std::chrono::seconds>(
      last_packet_tp_ - tos_tp_)};
    if (sec >= 30s) return;
    auto& packet{*end(tos_queue_)};
    auto const adr_high{adrHigh()};
    auto it{std::copy(cbegin(adr_high), cend(adr_high), begin(packet))};
    auto const adr_low{adrLow()};
    it = std::copy(cbegin(adr_low), cend(adr_low), it);
    auto const time{encode_datagram(
      make_datagram<Bits::_12>(0u, static_cast<uint32_t>(sec.count())))};
    std::copy(cbegin(time), cend(time), it);
    tos_queue_.push_back();
  }

  /// Logon enable
  ///
  /// \param  gg          Address group
  /// \param  cid         Command station ID
  /// \param  session_id  Session ID
  void logonEnable(uint8_t gg, uint16_t cid, uint8_t session_id) {
    auto const cid_equal{cid == cid_};
    auto const session_id_equal{session_id == session_id_};

    // Exceptions
    // - Either skip logon if diff between session IDs is <=4
    // - Or force new logon if diff is >4
    if (cid_equal && !session_id_equal) {
      logon_selected_ = logon_assigned_ = session_id - session_id_ <= 4u;
      logon_backoff_.reset();
    }

    cid_ = cid;
    session_id_ = session_id;

    if (logon_selected_ || logon_assigned_) return;
    switch (gg) {
      case 0b00u: [[fallthrough]];              // All decoders
      case 0b01u: break;                        // Multi-function decoders
      case 0b10u: return;                       // Accessory decoder
      case 0b11u: logon_backoff_.now(); break;  // No backoff
    }

    if (logon_backoff_) return;
    logon_queue_.push_back(encode_datagram(make_datagram<Bits::_48>(
      15u,
      static_cast<uint64_t>(zimo_id) << 32u | did_[0uz] << 24u |
        did_[1uz] << 16u | did_[2uz] << 8u | did_[3uz])));
  }

  /// Logon select
  ///
  /// \param  did Unique ID
  void logonSelect(std::span<uint8_t const, 4uz> did) {
    if (logon_assigned_ || !std::ranges::equal(did, did_)) return;
    logon_selected_ = true;
    std::array<uint8_t, 5uz> data{
      static_cast<uint8_t>(ztl::make_mask(7u) | (addrs_.primary >> 8u)),
      static_cast<uint8_t>(addrs_.primary),
      0u,
      0u,
      0u};
    logon_queue_.push_back(encode_datagram(make_datagram<Bits::_48>(
      static_cast<uint64_t>(data[0uz]) << 40uz |
      static_cast<uint64_t>(data[1uz]) << 32uz | data[2uz] << 24uz |
      data[3uz] << 16uz | data[4uz] << 8uz | crc8(data))));
  }

  /// Logon assign
  ///
  /// \param  did   Unique ID
  /// \param  addr  Assigned address
  void logonAssign(std::span<uint8_t const, 4uz> did, Address addr) {
    if (!std::ranges::equal(did, did_)) return;
    logon_assigned_ = logon_store_ = true;
    // Fucking stupid and doesn't conform to standard...
    addrs_.primary = addrs_.logon = addr;
    static constexpr std::array<uint8_t, 5uz> data{
      13u << 4u | 0u, 0u, 0u, 0u, 0u};
    logon_queue_.push_back(encode_datagram(make_datagram<Bits::_48>(
      static_cast<uint64_t>(data[0uz]) << 40uz |
      static_cast<uint64_t>(data[1uz]) << 32uz | data[2uz] << 24uz |
      data[3uz] << 16uz | data[4uz] << 8uz | crc8(data))));
  }

  // Shared with derived class
  Addresses addrs_{};

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
    std::copy(begin(dyn), end(dyn), begin(end(dyn_queue_)->data));
    end(dyn_queue_)->size = size(dyn);
    dyn_queue_.push_back();
  }

  /// Handle app:adr_low and app:adr_high datagrams
  void appAdr() {
    // TODO consist currently never sent!
    auto const adr_high{adrHigh()};
    ch1_ = ch1_ == adr_high ? adrLow() : adr_high;
    impl().transmitBiDi({cbegin(ch1_), size(ch1_)});
  }

  /// Handle app:pom, app:ext, app:dyn and app:subID datagrams
  void appPomExtDynSubId() {
    // Send either pom only (no fucking thanks ESU)
    if (!empty(pom_queue_)) appPom();
    // Or whatever fits into channel2
    else appExtDynSubId();
  }

  /// Handle app::pom
  void appPom() {
    auto const& packet{pom_queue_.front()};
    std::copy(cbegin(packet), cend(packet), begin(ch2_));
    impl().transmitBiDi({cbegin(ch2_), size(packet)});
    pom_queue_.pop_front();
  }

  /// Handle app:ext, app:dyn and app:subID datagrams
  void appExtDynSubId() {
    if (empty(dyn_queue_)) return;
    auto first{begin(ch2_)};
    auto const last{cend(ch2_)};
    do {
      auto const& packet{dyn_queue_.front()};
      first = std::copy_n(cbegin(packet.data), packet.size, first);
      dyn_queue_.pop_front();
    } while (!empty(dyn_queue_) && last - first >= dyn_queue_.front().size);
    impl().transmitBiDi({cbegin(ch2_), first});
  }

  /// Handle app:tos
  void appTos() {
    if (empty(tos_queue_)) return;
    auto const& packet{tos_queue_.front()};
    std::ranges::copy(packet, begin(ch2_));
    impl().transmitBiDi({begin(ch2_), size(packet)});
    tos_queue_.pop_front();
  }

  /// Handle app::logon
  void appLogon(uint32_t ch) {
    if (empty(logon_queue_)) return;
    if (auto const& packet{logon_queue_.front()}; ch == 1u) {
      std::copy(begin(packet), begin(packet) + 2, begin(ch1_));
      impl().transmitBiDi({cbegin(ch1_), size(ch1_)});
    } else {
      std::copy(begin(packet) + 2, end(packet), begin(ch2_));
      impl().transmitBiDi({cbegin(ch2_), size(ch2_)});
      logon_queue_.pop_front();
    }
  }

  /// Get app:adr_high
  ///
  /// \return Datagram for app:adr_high
  auto adrHigh() const {
    // TODO consist currently never sent!
    return encode_datagram(make_datagram<Bits::_12>(
      1u,
      addrs_.primary < 128u ? 0u : 0x80u | (addrs_.primary & 0x3F00u) >> 8u));
  }

  /// Get app:adr_low
  ///
  /// \return Datagram for app:adr_low
  auto adrLow() const {
    // TODO consist currently never sent!
    return encode_datagram(
      make_datagram<Bits::_12>(2u, addrs_.primary & 0x00FFu));
  }

  /// Logon store
  ///
  /// I fucking hate this. RCN218 stupidly requires us to answer extended
  /// packets directly in the following cutout. This is so time-critical that
  /// logon information can only be stored asynchronously...
  void logonStore() {
    if (!logon_store_) return;
    logon_store_ = false;
    // This is wrong on so many levels... I can't even... FML
    impl().writeCv(17u - 1u,
                   static_cast<uint8_t>(0b1100'0000u | (addrs_.primary >> 8u)));
    impl().writeCv(18u - 1u, static_cast<uint8_t>(addrs_.primary));
    impl().writeCv(29u - 1u, true, 5u);
    impl().writeCv(65297u - 1u, static_cast<uint8_t>(addrs_.logon >> 8u));
    impl().writeCv(65298u - 1u, static_cast<uint8_t>(addrs_.logon));
    impl().writeCv(65299u - 1u, static_cast<uint8_t>(cid_ >> 8u));
    impl().writeCv(65300u - 1u, static_cast<uint8_t>(cid_));
    impl().writeCv(65301u - 1u, session_id_);
  }

  /// Update time points
  ///
  /// In case time between two packets is >=2s allow tip-off search again.
  void updateTimepoints() {
    auto const packet_tp{std::chrono::system_clock::now()};
    if (packet_tp - last_packet_tp_ >= 2s) tos_tp_ = packet_tp;
    last_packet_tp_ = packet_tp;
  }

  std::chrono::time_point<std::chrono::system_clock> last_packet_tp_{};
  std::chrono::time_point<std::chrono::system_clock> tos_tp_{};
  std::array<uint8_t, 4uz> did_{};
  Channel1 ch1_{};
  Channel2 ch2_{};
  ztl::circular_array<Packet, DCC_RX_BIDI_QUEUE_SIZE> dyn_queue_{};
  ztl::circular_array<Channel1, DCC_RX_BIDI_QUEUE_SIZE> pom_queue_{};
  ztl::circular_array<Channel2, 2uz> tos_queue_{};
  ztl::circular_array<BundledChannels, 2uz> logon_queue_{};
  LogonBackoff logon_backoff_{};
  uint16_t cid_{};        ///< Central ID
  uint8_t session_id_{};  ///< Session ID
  uint8_t qos_{};         ///< Quality of service

  // Not bitfields as those are most likely mutated in interrupt context
  bool ch1_enabled_{};
  bool ch2_enabled_{};
  bool ch2_consist_enabled_{};
  bool logon_selected_{};
  bool logon_assigned_{};
  bool logon_store_{};
};

}  // namespace dcc::rx::bidi