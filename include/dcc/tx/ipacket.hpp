// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Indexed Packet buffer
///
/// \file   dcc/tx/ipacket.hpp
/// \author Jonas Gahlert
/// \date   19.03.2025

#pragma once

#include "../packet.hpp"
#include "config.hpp"
#include "timing.hpp"

#include <algorithm>

namespace dcc::tx {

/// Indexed \ref dcc::Packet "Packet" buffer, used for on-the-fly DCC timing
/// calculation.
///
/// \details
/// Packet is indexed and timings are returned half-bit-wise. The packet can be
/// cecked for remaining data with a call to \ref IPacket::hasNext "hasNext" and
/// a call to \ref IPacket::next "next" yields the next timing.
///
/// In case this is an idle-packet, the index packet can be reset with \ref
/// IPacket::resetIndex "resetIndex" and a new \ref dcc::tx::Config "Config" can
/// be loaded using \ref IPacket::recalculate "recalculate".
///
/// \warning
/// The Packet will assert an out-of-bounds state. Usage of \ref
/// IPacket::hasNext "hasNext" before calling \ref IPacket::next "next" is
/// strongly recommended.
struct IPacket {
  /// CTor - Packet copy
  ///
  /// \param p    Packet
  /// \param cfg  Config
  IPacket(Packet const& p, Config cfg = {}) : _packet{p}, _cfg{cfg} {}

  /// CTor - Byte span
  ///
  /// \param b    Bytes
  /// \param cfg  Config
  IPacket(std::span<uint8_t const> b, Config cfg = {}) : _cfg{cfg} {
    _packet.clear();
    std::copy(b.begin(), b.end(), std::back_inserter(_packet));
  }

  /// CTor - Idle Packet
  ///
  /// \param cfg  Config
  IPacket(Config cfg = {}) : _packet{make_idle_packet()}, _cfg{cfg} {}

  /// Next timing
  ///
  /// \return Next timing
  uint16_t next() {
    // Check that packet isn't out of bounds
    assert(_packet_index <= _packet.size());

    bool bit = false;
    if (_preamble_index < _cfg.num_preamble) {
      // Preamble
      bit = true;
      if (_half) _preamble_index++;

    } else {
      // Packet
      if (_packet_index < _packet.size()) {
        // Byte
        bit = (_packet[_packet_index] & (1 << _byte_index)) > 0;
        if (_half) {
          _byte_index--;
          if (_byte_index == static_cast<Packet::size_type>(-1)) {
            _packet_index++;
            _byte_index = 8;
          }
        }
      } else {
        // Stop bit
        bit = true;
        if (_half) _packet_index++;
      }
    }
    // Result
    _half = !_half;
    return (bit) ? _cfg.bit1_duration : _cfg.bit0_duration;
  }

  /// Has next
  ///
  /// \retval true  Has next
  /// \retval false Packet done
  bool hasNext() const { return (_packet.size() + 1) > _packet_index; }

  /// Reset index
  ///
  /// \note Intended for use with idle packet only
  void resetIndex() {
    _preamble_index = _packet_index = 0;
    _byte_index = 8;
  }

  /// Recalculate
  ///
  /// \note Intended for use with idle packet only
  ///
  /// \param cfg Config
  void recalculate(Config cfg = {}) { _cfg = cfg; }

private:
  /// Packet
  Packet _packet{0};

  /// Half bit indicator
  bool _half{false};

  /// Preamble index
  Packet::size_type _preamble_index{0};

  /// Packet index
  Packet::size_type _packet_index{0};

  /// Byte index
  Packet::size_type _byte_index{8};

  /// Config
  Config _cfg;
};

} // namespace dcc::tx