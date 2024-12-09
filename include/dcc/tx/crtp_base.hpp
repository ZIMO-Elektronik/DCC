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

namespace dcc::tx {

/// CRTP base for transmitting DCC
///
/// \tparam T Type to downcast to
template<typename T>
struct CrtpBase {
  friend T;

  /// Initialize
  ///
  /// \param  cfg Configuration
  void init(Config cfg) {
    assert(cfg.num_preamble >= DCC_TX_MIN_PREAMBLE_BITS &&                 //
           cfg.num_preamble <= DCC_TX_MAX_PREAMBLE_BITS &&                 //
           cfg.bit1_duration >= Bit1Min && cfg.bit1_duration <= Bit1Max && //
           cfg.bit0_duration >= Bit0Min && cfg.bit0_duration <= Bit0Max);  //
    _cfg = cfg;
  }

  /// Transmit packet
  ///
  /// \param  packet  Packet
  void packet(Packet const& packet) {
    return bytes({cbegin(packet), size(packet)});
  }

  /// Transmit bytes
  ///
  /// \param  bytes Bytes
  void bytes(std::span<uint8_t const> bytes) {
    if (full(_deque)) return;
    assert(size(bytes) <= DCC_MAX_PACKET_SIZE);
    _deque.push_back(bytes2timings(bytes, _cfg));
  }

  /// Get next bit duration to transmit in µs
  ///
  /// \return Bit duration in µs
  Timings::value_type transmit() {
    // As long as there are packet timings
    if (_packet_count < _packet->size()) return packetTiming();
    // or BiDi timings
    else if (_cfg.flags.bidi && _bidi_count <= 4uz) return bidiTiming();

    /// \todo theoretically deque could be popped here safely?
    /// we'd just need to check whether packet doesn't point to idle_packet and
    /// deque ain't empty?

    // Deque is empty, send idle packet
    if (empty(_deque)) _packet = &_idle_packet;
    // Deque contains packet, send it
    else {
      _packet = &_deque.front();
      // Careful! This only works because of the design of ztl::inplace_deque.
      // The slot _packet currently points to will stay valid until the next
      // call of pop_front().
      _deque.pop_front();
    }
    _packet_count = _bidi_count = 0uz;

    return packetTiming();
  }

private:
  constexpr CrtpBase() = default;
  CommandStation auto& impl() { return static_cast<T&>(*this); }
  CommandStation auto const& impl() const {
    return static_cast<T const&>(*this);
  }

  /// Packet timing
  ///
  /// \return Next timings from current packet
  Timings::value_type packetTiming() {
    auto const retval{
      (*_packet)[static_cast<Timings::size_type>(_packet_count)]};
    // First half bit
    if (!(_packet_count++ % 2uz))
      impl().trackOutputs(false ^ _cfg.flags.invert, true ^ _cfg.flags.invert);
    // Second half bit
    else
      impl().trackOutputs(true ^ _cfg.flags.invert, false ^ _cfg.flags.invert);
    return retval;
  }

  /// BiDi timing
  ///
  /// \return Next BiDi timing
  Timings::value_type bidiTiming() {
    switch (_bidi_count++) {
      // Send half a 1 bit
      case 0uz:
        impl().trackOutputs(false ^ _cfg.flags.invert,
                            true ^ _cfg.flags.invert);
        return static_cast<Timings::value_type>(bidi::Timing::TCS);

      // Cutout start
      case 1uz:
        impl().trackOutputs(false, false);
        impl().biDiStart();
        return static_cast<Timings::value_type>(bidi::Timing::TTS1 -
                                                bidi::Timing::TCS);

      // Channel1 start
      case 2uz:
        impl().biDiChannel1();
        return static_cast<Timings::value_type>(bidi::Timing::TTS2 -
                                                bidi::Timing::TTS1);

      // Channel2 start
      case 3uz:
        impl().biDiChannel2();
        return static_cast<Timings::value_type>(bidi::Timing::TTC2 -
                                                bidi::Timing::TTS2);

      // Cutout end
      default:
        impl().biDiEnd();
        return static_cast<Timings::value_type>(bidi::Timing::TCE -
                                                bidi::Timing::TTC2);
    }
  }

  static constexpr Timings _idle_packet{packet2timings(make_idle_packet())};

  ztl::inplace_deque<Timings, DCC_TX_DEQUE_SIZE> _deque{};
  Timings const* _packet{&_idle_packet};
  size_t _packet_count{};
  size_t _bidi_count{};
  Config _cfg{};
};

} // namespace dcc::tx