// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Transmit base
///
/// \file   dcc/tx/crtp_base.hpp
/// \author Vincent Hamp
/// \date   12/06/2022

#pragma once

#include <concepts>
#include <span>
#include <ztl/inplace_deque.hpp>
#include "../bidi/datagram.hpp"
#include "../bidi/timing.hpp"
#include "command_station.hpp"
#include "config.hpp"
#include "timings.hpp"
#include "timings_adapter.hpp"

namespace dcc::tx {

/// CRTP base for transmitting DCC
///
/// \tparam T Type to downcast to
/// \tparam D Deque value type
template<typename T, typename D = Packet>
requires(std::same_as<D, Packet> || std::same_as<D, Timings>)
struct CrtpBase {
  friend T;

  using value_type =
    std::conditional_t<std::same_as<D, Packet>, TimingsAdapter, Timings>;

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
    if constexpr (std::same_as<D, Packet>)
      _idle_packet = TimingsAdapter{packet, _cfg};
    else if constexpr (std::same_as<D, Timings>)
      _idle_packet = bytes2timings(packet, _cfg);
    _first = begin(_idle_packet);
    _last = cend(_idle_packet);
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
  /// \return Bit duration in µs
  Timings::value_type transmit() {
    // Packet timings
    if (_first != _last) return packetTiming();
    // Packet end
    else if constexpr (requires(T t) {
                         { t.packetEnd() };
                       })
      if (!_bidi_count) impl().packetEnd();

    // BiDi timings
    if (_cfg.flags.bidi && _bidi_count <= 4uz) return biDiTiming();
    else _bidi_count = 0uz;

    // Deque is empty, transmit idle packet
    if (empty(_deque)) {
      _first = begin(_idle_packet);
      _last = cend(_idle_packet);
    }
    // Deque contains packet, transmit it
    else {
      _first = begin(_deque.front());
      _last = cend(_deque.front());
      /// \warning
      /// Careful! This only works because of the design of ztl::inplace_deque.
      /// The element currently pointed to will stay valid until the next call
      /// of pop_front().
      _deque.pop_front();
    }
    return packetTiming();
  }

  /// Get deque size
  ///
  /// \return Deque size
  constexpr auto size() const { return std::size(_deque); }

  /// Get deque capacity
  ///
  /// \return Deque capacity
  constexpr auto capacity() const { return DCC_TX_DEQUE_SIZE; }

private:
  constexpr CrtpBase() = default;
  auto& impl() { return static_cast<T&>(*this); }
  auto const& impl() const { return static_cast<T const&>(*this); }

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
  Timings::value_type biDiTiming() {
    switch (_bidi_count++) {
      // Send half a 1 bit
      case 0uz:
        toggleTrackOutputs();
        return static_cast<Timings::value_type>(bidi::Timing::TCS);

      // Cutout start
      case 1uz:
        toggleTrackOutputs();
        if constexpr (requires(T t) {
                        { t.biDiStart() };
                      })
          impl().biDiStart();
        return static_cast<Timings::value_type>(bidi::Timing::TTS1 -
                                                bidi::Timing::TCS);

      // Channel 1 start
      case 2uz:
        if constexpr (requires(T t) {
                        { t.biDiChannel1() };
                      })
          impl().biDiChannel1();
        return static_cast<Timings::value_type>(bidi::Timing::TTS2 -
                                                bidi::Timing::TTS1);

      // Channel 2 start
      case 3uz:
        if constexpr (requires(T t) {
                        { t.biDiChannel2() };
                      })
          impl().biDiChannel2();
        return static_cast<Timings::value_type>(bidi::Timing::TTC2 -
                                                bidi::Timing::TTS2);

      // Cutout end
      default:
        if constexpr (requires(T t) {
                        { t.biDiEnd() };
                      })
          impl().biDiEnd();
        return static_cast<Timings::value_type>(bidi::Timing::TCE -
                                                bidi::Timing::TTC2);
    }
  }

  /// Add packet or timings to deque
  ///
  /// \param  bytes Bytes containing DCC packet
  void pushBack(std::span<uint8_t const> bytes) {
    if constexpr (std::same_as<D, Packet>) _deque.push_back({bytes, _cfg});
    else if constexpr (std::same_as<D, Timings>)
      _deque.push_back(bytes2timings(bytes, _cfg));
  }

  /// Toggle track outputs
  void toggleTrackOutputs() {
    if constexpr (requires(T t, bool N, bool P) {
                    { t.trackOutputs(N, P) };
                  }) {
      // By default the phase is "positive", so P > N for the first half bit.
      impl().trackOutputs(_polarity, !_polarity);
      _polarity = !_polarity;
    }
  }

  /// Deque
  ztl::inplace_deque<value_type, DCC_TX_DEQUE_SIZE> _deque{};

  /// Idle packet
  value_type _idle_packet{};

  /// Iterators
  decltype(std::begin(_idle_packet)) _first{std::begin(_idle_packet)};
  decltype(std::cend(_idle_packet)) _last{std::cend(_idle_packet)};

  size_t _bidi_count{}; ///< Count BiDi timings
  Config _cfg{};        ///< Configuration
  bool _polarity{};     ///< Track polarity
};

} // namespace dcc::tx
