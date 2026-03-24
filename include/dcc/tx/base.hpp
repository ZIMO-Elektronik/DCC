// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Transmit base
///
/// \file   dcc/tx/crtp_base.hpp
/// \author Vincent Hamp
/// \date   12/06/2022

#pragma once

#include <cassert>
#include <concepts>
#include <span>
#include <utility>
#include <ztl/inplace_deque.hpp>
#include "../bidi/datagram.hpp"
#include "../bidi/timing.hpp"
#include "../utility.hpp"
#include "addresses.hpp"
#include "bidi_phase.hpp"
#include "command_station.hpp"
#include "config.hpp"
#include "timings.hpp"
#include "timings_adapter.hpp"

namespace dcc::tx {

/// Base for transmitting DCC
///
/// \tparam D Deque value type
template<typename D = Packet>
requires(std::same_as<D, Packet> || std::same_as<D, Timings>)
struct Base {
  using value_type = std::pair<
    Address,
    std::conditional_t<std::same_as<D, Packet>, TimingsAdapter, Timings>>;

  /// Initialize
  ///
  /// \param  cfg     Configuration
  /// \param  packet  Packet to send as soon as deque is empty
  void init(Config cfg = {}, Packet const& packet = make_idle_packet()) {
    assert(cfg.num_preamble >= DCC_TX_MIN_PREAMBLE_BITS &&                 //
           cfg.num_preamble <= DCC_TX_MAX_PREAMBLE_BITS &&                 //
           cfg.bit1_duration >= Bit1Min && cfg.bit1_duration <= Bit1Max && //
           cfg.bit0_duration >= Bit0Min && cfg.bit0_duration <= Bit0Max);  //
    _cfg = cfg;
    _idle_packet.first = _addrs.current = decode_address(packet);
    if constexpr (std::same_as<D, Packet>)
      _idle_packet.second = TimingsAdapter{packet, _cfg};
    else if constexpr (std::same_as<D, Timings>)
      _idle_packet.second = bytes2timings(packet, _cfg);
    _first = begin(_idle_packet.second);
    _last = cend(_idle_packet.second);
    _idle = true;
  }

  /// Transmit packet
  ///
  /// \param  packet  Packet
  /// \retval true    Packet added to deque
  /// \retval false   Packet not added to deque
  bool packet(Packet const& packet) {
    return bytes({cbegin(packet), std::size(packet)});
  }

  /// Transmit bytes
  ///
  /// \param  bytes Bytes containing DCC packet
  /// \retval true  Bytes added to deque
  /// \retval false Bytes not added to deque
  bool bytes(std::span<uint8_t const> bytes) {
    if (full(_deque)) return false;
    assert(std::size(bytes) <= DCC_MAX_PACKET_SIZE);
    pushBack(bytes);
    return true;
  }

  /// Get next bit duration to transmit in µs
  ///
  /// \tparam Self self
  /// \return Bit duration in µs
  template<typename Self>
  Timings::value_type transmit(this Self&& self) {
    // Packet timings
    if (self._first != self._last) return self.packetTiming();

    // Packet end
    if constexpr (requires(Self&& self) {
                    { self.packetEnd() };
                  })
      if (self._bidi_phase == detail::BiDiPhase::Pre) self.packetEnd();

    // BiDi timings
    if (self._cfg.flags.bidi && self._bidi_phase != detail::BiDiPhase::Done)
      return self.biDiTiming();
    else self._bidi_phase = detail::BiDiPhase::Pre;

    // Only pop if packet came from deque
    if (!self._idle) {
      assert(!empty(self._deque));
      self._deque.pop_front();
    }

    // Next packet
    self._idle = empty(self._deque);
    auto& packet{self._idle ? self._idle_packet : self._deque.front()};
    self._addrs.last = self._addrs.current;
    self._addrs.current = packet.first;
    self._first = begin(packet.second);
    self._last = cend(packet.second);
    return self.packetTiming();
  }

  /// Get deque size
  ///
  /// \return Deque size
  constexpr auto size() const { return std::size(_deque); }

  /// Get deque capacity
  ///
  /// \return Deque capacity
  constexpr auto capacity() const { return DCC_TX_DEQUE_SIZE; }

  /// Get address of last transmission
  ///
  /// \return Address of last transmission
  constexpr auto address() const { return _addrs.last; }

protected:
  constexpr Base() = default;

  /// Packet timing
  ///
  /// \return Next timings from current packet
  Timings::value_type packetTiming() {
    toggleTrackOutputs();
    auto const retval{*_first};
    ++_first;
    return retval;
  }

  /// BiDi timing
  ///
  /// \return Next BiDi timing
  template<typename Self>
  Timings::value_type biDiTiming(this Self&& self) {
    switch (self._bidi_phase++) {
      // Send half a 1 bit
      case detail::BiDiPhase::Pre:
        self.toggleTrackOutputs();
        return static_cast<Timings::value_type>(bidi::Timing::TCS);

      // Cutout start
      case detail::BiDiPhase::Start:
        self.toggleTrackOutputs();
        if constexpr (requires(Self self) {
                        { self.biDiStart() };
                      })
          self.biDiStart();
        return static_cast<Timings::value_type>(bidi::Timing::TTS1 -
                                                bidi::Timing::TCS);

      // Channel 1 start
      case detail::BiDiPhase::Channel1:
        if constexpr (requires(Self self) {
                        { self.biDiChannel1() };
                      })
          self.biDiChannel1();
        return static_cast<Timings::value_type>(bidi::Timing::TTS2 -
                                                bidi::Timing::TTS1);

      // Channel 2 start
      case detail::BiDiPhase::Channel2:
        if constexpr (requires(Self self) {
                        { self.biDiChannel2() };
                      })
          self.biDiChannel2();
        return static_cast<Timings::value_type>(bidi::Timing::TTC2 -
                                                bidi::Timing::TTS2);

      // Cutout end
      default:
        if constexpr (requires(Self self) {
                        { self.biDiEnd() };
                      })
          self.biDiEnd();
        return static_cast<Timings::value_type>(bidi::Timing::TCE -
                                                bidi::Timing::TTC2);
    }
  }

  /// Add packet or timings to deque
  ///
  /// \param  bytes Bytes containing DCC packet
  void pushBack(std::span<uint8_t const> bytes) {
    if constexpr (std::same_as<D, Packet>)
      _deque.push_back({decode_address(bytes), {bytes, _cfg}});
    else if constexpr (std::same_as<D, Timings>)
      _deque.push_back({decode_address(bytes), bytes2timings(bytes, _cfg)});
  }

  /// Toggle track outputs
  template<typename Self>
  void toggleTrackOutputs(this Self&& self) {
    if constexpr (requires(Self self, bool N, bool P) {
                    { self.trackOutputs(N, P) };
                  }) {
      // By default the phase is "positive", so P > N for the first half bit.
      self.trackOutputs(self._polarity, !self._polarity);
      self._polarity = !self._polarity;
    }
  }

  /// Deque
  ztl::inplace_deque<value_type, DCC_TX_DEQUE_SIZE> _deque{};

  /// Idle packet
  value_type _idle_packet{};

  /// Addresses
  Addresses _addrs{};

  /// Iterators
  decltype(std::begin(_idle_packet.second)) _first{
    std::begin(_idle_packet.second)};
  decltype(std::cend(_idle_packet.second)) _last{
    std::cend(_idle_packet.second)};

  detail::BiDiPhase _bidi_phase{}; ///< Count BiDi timings
  Config _cfg{};           ///< Configuration
  bool _polarity{};        ///< Track polarity
  bool _idle{true};        ///< Idle flag
};

} // namespace dcc::tx
