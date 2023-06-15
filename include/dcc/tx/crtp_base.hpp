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
#include <ztl/circular_array.hpp>
#include "../bidi/datagram.hpp"
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
    assert(cfg.preamble_bits >= 17u && cfg.preamble_bits <= 100u &&
           cfg.bit1_duration >= Bit1Min && cfg.bit1_duration <= Bit1Max &&
           cfg.bit0_duration >= Bit0Min && cfg.bit0_duration <= Bit0Max);
    cfg_ = cfg;
  }

  /// Transmit packet
  ///
  /// \param  packet  Packet
  void packet(Packet const& packet) {
    return raw({cbegin(packet.data), packet.size});
  }

  /// Transmit raw data
  ///
  /// \param  chunk Raw data
  void raw(std::span<uint8_t const> chunk) {
    if (queue_.full()) return;
    assert(size(chunk) <= DCC_MAX_PACKET_SIZE);
    queue_.push_back(raw2timings(chunk, cfg_));
  }

  /// Get next bit duration to transmit in µs
  ///
  /// \return Bit duration in µs
  Timings::value_type transmit() {
    // As long as there are packet timings
    if (packet_count_ < packet_->size) return packetTiming();
    // or BiDi timings
    else if (cfg_.bidi && bidi_count_ <= 4uz) return bidiTiming();

    // TODO theoretically queue could be popped here safely?
    // we'd just need to check whether packet doesn't point to idle_packet and
    // queue ain't empty?

    // Queue is empty, send idle packet
    if (empty(queue_)) packet_ = &idle_packet_;
    // Queue contains packet, send it
    else {
      packet_ = &queue_.front();
      // Careful! This only works because of the design of ztl::circular_array.
      // The slot packet_ currently points to will stay valid until the next
      // call of pop_front().
      queue_.pop_front();
    }
    packet_count_ = bidi_count_ = 0uz;

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
    auto const retval{packet_->values[packet_count_]};
    if (packet_count_++ % 2uz) impl().setTrackOutputs(false, true);
    else impl().setTrackOutputs(true, false);
    return retval;
  }

  /// BiDi timing
  ///
  /// \return Next BiDi timing
  Timings::value_type bidiTiming() {
    switch (bidi_count_++) {
      // Send half a 1 bit
      case 0uz:
        impl().setTrackOutputs(true, false);
        return static_cast<Timings::value_type>(Timing::BiDiTCS);

      // Cutout start
      case 1uz:
        impl().setTrackOutputs(false, false);
        impl().biDiStart();
        return static_cast<Timings::value_type>(Timing::BiDiTTS1 -
                                                Timing::BiDiTCS);

      // Channel1 start
      case 2uz:
        impl().biDiChannel1();
        return static_cast<Timings::value_type>(Timing::BiDiTTS2 -
                                                Timing::BiDiTTS1);

      // Channel2 start
      case 3uz:
        impl().biDiChannel2();
        return static_cast<Timings::value_type>(Timing::BiDiTTC2 -
                                                Timing::BiDiTTS2);

      // Cutout end
      default:
        impl().biDiEnd();
        return static_cast<Timings::value_type>(Timing::BiDiTCE -
                                                Timing::BiDiTTC2);
    }
  }

  static constexpr Timings idle_packet_{packet2timings(make_idle_packet())};

  ztl::circular_array<Timings, DCC_TX_QUEUE_SIZE> queue_{};  ///< Task queue
  Timings const* packet_{&idle_packet_};
  size_t packet_count_{};
  size_t bidi_count_{};
  Config cfg_{};
};

}  // namespace dcc::tx