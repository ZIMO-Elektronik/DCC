// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Indexed Timings buffer
///
/// \file   dcc/tx/itimings.hpp
/// \author Jonas Gahlert
/// \date   19.03.2025

#pragma once

#include <ztl/inplace_vector.hpp>
#include "../packet.hpp"
#include "config.hpp"
#include "timings.hpp"

namespace dcc::tx {

/// Indexed \ref dcc::tx::Timings "Timings" buffer, used for on-the-fly DCC
/// timing calculation.
///
/// \details
/// Packet is indexed and timings are returned half-bit-wise. The packet can be
/// cecked for remaining data with a call to \ref ITimings::hasNext "hasNext"
/// and a call to \ref ITimings::next "next" yields the next timing.
///
/// In case this is an idle-packet, the index packet can be reset with \ref
/// ITimings::resetIndex "resetIndex" and a new \ref dcc::tx::Config "Config"
/// can be loaded using \ref ITimings::recalculate "recalculate".
///
/// \warning
/// The Packet will assert an out-of-bounds state. Usage of \ref
/// ITimings::hasNext "hasNext" before calling \ref ITimings::next "next" is
/// strongly recommended.
struct ITimings {
  /// CTor - Packet copy
  ///
  /// \param p    Packet
  /// \param cfg  Config
  ITimings(Packet const& p, Config cfg = {})
    : _timings{packet2timings(p, cfg)} {}

  /// CTor - Byte span
  ///
  /// \param b    Bytes
  /// \param cfg  Config
  ITimings(std::span<uint8_t const> b, Config cfg = {})
    : _timings{bytes2timings(b, cfg)} {}

  /// CTor - Idle Packet
  ///
  /// \param cfg  Config
  ITimings(Config cfg = {})
    : _timings{bytes2timings(make_idle_packet(), cfg)} {}

  /// Next timing
  ///
  /// \return Next timing
  Timings::value_type next() {
    // Check that packet isn't out of bounds
    assert(_packet_index < _timings.size());

    return _timings[static_cast<Timings::size_type>(_packet_index++)];
  }

  /// Has next
  ///
  /// \retval true  Has next
  /// \retval false Packet done
  bool hasNext() const { return _packet_index < _timings.size(); }

  /// Reset index
  ///
  /// \note Intended for use with idle packet only
  void resetIndex() { _packet_index = 0; }

  /// Recalculate
  ///
  /// \note Intended for use with idle packet only
  ///
  /// \param cfg Config
  void recalculate(Config cfg = {}) {
    _timings = packet2timings(make_idle_packet(), cfg);
  }

private:
  /// Timings
  Timings _timings;

  /// Packet index
  size_t _packet_index{0};
};

} // namespace dcc::tx