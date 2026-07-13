// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive base
///
/// \file   dcc/rx/crtp_base.hpp
/// \author Vincent Hamp
/// \date   04/01/2022

#pragma once

#include <cassert>
#include <chrono>
#include <concepts>
#include <span>
#include <ztl/bits.hpp>
#include <ztl/inplace_deque.hpp>
#include "../bidi/acks.hpp"
#include "../bidi/channel.hpp"
#include "../bidi/datagram.hpp"
#include "../bidi/direction_status_byte.hpp"
#include "../bidi/kmh.hpp"
#include "../bidi/nak.hpp"
#include "../bidi/temperature.hpp"
#include "../bidi/track_voltage.hpp"
#include "../bidi/utility.hpp"
#include "../crc8.hpp"
#include "../direction.hpp"
#include "../instruction.hpp"
#include "../packet.hpp"
#include "../speed.hpp"
#include "../utility.hpp"
#include "addresses.hpp"
#include "async_readable.hpp"
#include "async_writable.hpp"
#include "backoff.hpp"
#include "decoder.hpp"
#include "east_west.hpp"
#include "timing.hpp"

namespace dcc::rx {

/// CRTP base for receiving DCC
///
/// \tparam T Type to downcast to
template<typename T>
struct CrtpBase {
  friend T;

  /// Initialize
  void init() {
    // Primary address
    auto const cv29{impl().readCv(29u - 1u)};
    if (cv29 & ztl::mask<5u>) {
      std::array const cv17_cv18{impl().readCv(17u - 1u),
                                 impl().readCv(18u - 1u)};
      _addrs.primary = decode_address(cv17_cv18);
    } else {
      auto const cv1{impl().readCv(1u - 1u)};
      _addrs.primary = decode_address(&cv1);
    }
    _addrs.primary.reversed = cv29 & ztl::mask<0u>;

    // Consist address
    auto const cv19{impl().readCv(19u - 1u)};
    auto const cv20{impl().readCv(20u - 1u)};
    auto const consist_addr{100u * (cv20 & 0b0111'1111u) +
                            (cv19 & 0b0111'1111u)};
    _addrs.consist = {static_cast<Address::value_type>(consist_addr),
                      consist_addr <= 127u ? Address::BasicLoco
                                           : Address::ExtendedLoco};
    _addrs.consist.reversed = cv19 & ztl::mask<7u>;

    // Legacy exception for F0
    _f0_exception = !(cv29 & ztl::mask<1u>);

    // Decoder lock
    auto const cv15{impl().readCv(15u - 1u)};
    auto const cv16{impl().readCv(16u - 1u)};
    _cvs_locked = cv15 != cv16 && cv15 && cv16;

    // BiDi
    auto const bidi_enabled{static_cast<bool>(cv29 & ztl::mask<3u>)};
    auto const ch2_consist_enabled{static_cast<bool>(cv20 & ztl::mask<7u>)};
    auto const cv28{impl().readCv(28u - 1u)};
    _ch1_addr_enabled = bidi_enabled && (cv28 & ztl::mask<0u>);
    _ch2_data_enabled = bidi_enabled && (cv28 & ztl::mask<1u>);
    _logon_enabled = bidi_enabled && (cv28 & ztl::mask<7u>);
    _ch2_consist_enabled = bidi_enabled && ch2_consist_enabled;

    // IDs
    _ids.decoder = {impl().readCv(DCC_RX_LOGON_DID_CV_ADDRESS + 0u),
                    impl().readCv(DCC_RX_LOGON_DID_CV_ADDRESS + 1u),
                    impl().readCv(DCC_RX_LOGON_DID_CV_ADDRESS + 2u),
                    impl().readCv(DCC_RX_LOGON_DID_CV_ADDRESS + 3u)};
    _ids.cs.front() = static_cast<decltype(_ids.cs)::value_type>(
      static_cast<uint32_t>(impl().readCv(DCC_RX_LOGON_CID_CV_ADDRESS + 0u))
        << 8u |
      impl().readCv(DCC_RX_LOGON_CID_CV_ADDRESS + 1u));
    _ids.session.front() = impl().readCv(DCC_RX_LOGON_SID_CV_ADDRESS);

    // Logon address
    std::array const logon_addr_cvs{
      impl().readCv(DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 0u),
      impl().readCv(DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 1u)};
    _addrs.logon = decode_address(logon_addr_cvs);

    // Initialization time point
    _tps.init = std::chrono::system_clock::now();

    // Clear deques
    _deques.dyn.clear();
    _deques.logon.clear();
    _deques.search.clear();
    _deques.adr.clear();
    _deques.pom.clear();
  }

  /// Enable
  void enable() {
    if (_enabled) return;
    _enabled = true;
  }

  /// Disable
  void disable() {
    if (!_enabled) return;
    _enabled = false;
    reset();
  }

  /// Encoding of commands bit by bit
  ///
  /// \param  time  Time in µs
  void receive(uint32_t time) {
    // Whatever we got, its not packet end anymore
    _packet_end = false;

    // Count consecutive one bits to determine if preamble is valid
    auto const bit{time2bit(time)};
    bool const valid_preamble{_counts.one_bit >=
                              DCC_RX_MIN_PREAMBLE_BITS * 2uz};
    _counts.one_bit = bit == _1 ? (_counts.one_bit + 1uz) : 0uz;

    // Reset if bit invalid
    if (bit == Invalid) return reset();

    // Alternate halfbit <-> bit
    if (_state > Startbit && (_is_halfbit = !_is_halfbit)) return;

    // Successfully received a bit
    switch (_state) {
      case Preamble:
        if (bit) return;
        else if (valid_preamble) _state = Startbit;
        else return reset();
        break;

      case Startbit:
        _packets.current.clear();
        _counts.bit = 0uz;
        _is_halfbit = false;
        ++_counts.preamble;
        _state = Data;
        break;

      case Data:
        _byte = static_cast<uint8_t>((_byte << 1u) | bit);
        if (++_counts.bit < CHAR_BIT) return;
        _packets.current.push_back(_byte);
        _checksum = static_cast<uint8_t>(_checksum ^ _byte);
        _counts.bit = _byte = 0u;
        _state = Endbit;
        break;

      case Endbit:
        if (!bit) {
          _state = Data;
          return;
        }
        // Execute or push back valid packet
        else if (!_checksum && size(_packets.current) >= 3uz) {
          _packet_end = true;
          ++_counts.packet;
          _addrs.received = decode_address(_packets.current);
          _instr = decode_instruction(_packets.current);
          if (!executeHandlerMode() && !full(_deques.packet))
            _deques.packet.push_back(_packets.current);
        }
        // Immediately clear received address and invalid packet
        else {
          _addrs.received = {};
          _packets.current.clear();
        }
        reset();
    }
  }

  /// Execute received commands
  ///
  /// \retval true  Command executed
  /// \retval false Command not executed
  bool execute() { return executeThreadMode(); }

  /// Service mode
  ///
  /// \retval true  Service mode active
  /// \retval false Operations mode active
  bool serviceMode() const { return _mode == Service; }

  /// MAN function
  ///
  /// \retval true  MAN function active
  /// \retval false MAN function inactive
  bool man() const { return _man; }

  /// Check if last received bit was a packet end
  ///
  /// \retval true  Last received bit was packet end
  /// \retval false Last received bit wasn't packet end
  bool packetEnd() const { return _packet_end; }

  /// Addresses
  ///
  /// \return Addresses
  Addresses const& addresses() const { return _addrs; }

  /// Add dyn (ID7) datagrams to deque
  ///
  /// \tparam Dyns... Types of dyn datagrams
  /// \param  dyns... Datagrams
  template<std::derived_from<bidi::app::Dyn>... Dyns>
  void datagram(Dyns&&... dyns) {
    // Block full and release empty deque to avoid getting the same datagrams
    // send over and over again...
    if (full(_deques.dyn)) _block_dyn_deque = true;
    else if (empty(_deques.dyn)) {
      _block_dyn_deque = false;
      dyn(_qos, 7u);
    }

    // Only allow pushing datagrams if not blocked
    if (!_block_dyn_deque) (dyn(dyns.d, dyns.x), ...);
  }

  /// Start channel1 (12 bit payload)
  void biDiChannel1() {
    if (!packetEnd()) return;
    switch (_addrs.received.type) {
      case Address::BasicLoco: [[fallthrough]];
      case Address::ExtendedLoco: appAdr(); break;
      case Address::AutomaticLogon: appLogon(1u); break;
      default: break;
    }
  }

  /// Start channel2 (36 bit payload)
  void biDiChannel2() {
    if (!packetEnd()) return;
    switch (_addrs.received.type) {
      case Address::Broadcast: appSearch(); break;
      case Address::BasicLoco: [[fallthrough]];
      case Address::ExtendedLoco:
        if (_addrs.received ==
            (_logon_assigned ? _addrs.logon : _addrs.primary)) {
          if (_instr == Instruction::CvAccess)
            !empty(_deques.pom) ? appPom() : appXpom();
          else if (!empty(_deques.pom)) appPom();
          else if (!empty(_deques.xpom)) appXpom();
          else appDyn();
        } else if (_addrs.received == _addrs.consist && _ch2_consist_enabled)
          appDyn();
        break;
      case Address::AutomaticLogon: appLogon(2u); break;
      default: break;
    }
  }

private:
  constexpr CrtpBase() = default;
  Decoder auto& impl() { return static_cast<T&>(*this); }
  Decoder auto const& impl() const { return static_cast<T const&>(*this); }

  /// Execute in handler mode (interrupt context)
  ///
  /// \retval true  Command executed
  /// \retval false Command not executed
  bool executeHandlerMode() {
    if (_addrs.received.type == Address::AutomaticLogon)
      return executeOperations(_addrs.received, _packets.current, true);
    else return false;
  }

  /// Execute in thread mode
  ///
  /// \retval true  Command executed
  /// \retval false Command not executed
  bool executeThreadMode() {
    if (packetEnd() || empty(_deques.packet)) return false;
    adr();              // Prepare address broadcasts for BiDi channel 1
    logonStore();       // Store logon information if necessary
    updateQos();        // Update quality of service
    updateTimePoints(); // Update time points for tip-off search
    auto const& packet{_deques.packet.front()};
    auto const addr{decode_address(packet)};
    auto const retval{serviceMode() ? executeService()
                                    : executeOperations(addr, packet)};
    _deques.packet.pop_front();
    return retval;
  }

  /// Execute commands in operations mode
  ///
  /// \param  addr          Address
  /// \param  packet        Packet
  /// \param  handler_mode  Handler mode
  /// \retval true    Command executed
  /// \retval false   Command not executed
  bool executeOperations(Address addr,
                         Packet const& packet,
                         bool handler_mode = false) {
    switch (addr.type) {
      case Address::Broadcast: [[fallthrough]];
      case Address::BasicLoco:
        return executeOperationsAddressed(addr,
                                          {cbegin(packet) + 1, cend(packet)});
      case Address::ExtendedLoco:
        return executeOperationsAddressed(addr,
                                          {cbegin(packet) + 2, cend(packet)});
      case Address::AutomaticLogon:
        if (size(packet) <= (6uz + sizeof(_checksum)) || !crc8(packet))
          return executeAutomaticLogon({cbegin(packet) + 1, cend(packet)},
                                       handler_mode);
        [[fallthrough]];
      default: return false;
    }
  }

  /// Execute commands in service mode
  ///
  /// \retval true
  bool executeService() {
    // Count own equal packets (required for CV access)
    countOwnEqualPackets();

    // Reset
    if (auto const& packet{_deques.packet.front()}; !packet[0uz])
      ;
    // Exit
    else if ((packet[0uz] & 0xF0u) != 0b0111'0000u) serviceMode(false);
    // Register mode
    else if (size(packet) == 3uz) registerMode({cbegin(packet), cend(packet)});
    // CV access
    else if (size(packet) == 4uz && _counts.equal_packets == 2uz)
      executeCvAccessLong(0u, {cbegin(packet), cend(packet)});

    return true;
  }

  /// Execute addressed commands
  ///
  /// \param  addr  Address
  /// \param  bytes Raw bytes
  /// \retval true  Command executed
  /// \retval false Command not executed
  bool executeOperationsAddressed(Address addr,
                                  std::span<uint8_t const> bytes) {
    // Address is broadcast
    if (!addr)
      ;
    // Address is primary or consist and logon ain't assigned
    else if ((addr == _addrs.primary || addr == _addrs.consist) &&
             !_logon_assigned)
      ;
    // Address is logon and logon assigned, pretend it's primary from here on
    else if (addr == _addrs.logon && _logon_assigned) addr = _addrs.primary;
    // Address is not of interest
    else return false;

    // Count own equal packets (required for CV access)
    countOwnEqualPackets();

    switch (decode_instruction(bytes)) {
      case Instruction::DecoderControl:
        if (!addr && !bytes[0uz]) {
          serviceMode(true);
          return true;
        } else return executeDecoderControl(bytes);
      case Instruction::ConsistControl: return executeConsistControl(bytes);
      case Instruction::AdvancedOperations:
        return executeAdvancedOperations(addr, bytes);
      case Instruction::SpeedDirection:
        return executeSpeedDirection(addr, bytes);
      case Instruction::FunctionGroup: return executeFunctionGroup(addr, bytes);
      case Instruction::FeatureExpansion:
        return executeFeatureExpansion(addr, bytes);
      case Instruction::CvAccess: return executeCvAccess(addr, bytes);
      default: return false;
    }
  }

  /// Execute automatic logon
  ///
  /// \param  bytes         Raw bytes
  /// \param  handler_mode  Handler mode
  /// \retval true  Command executed
  /// \retval false Command not executed
  bool executeAutomaticLogon(std::span<uint8_t const> bytes,
                             bool handler_mode) {
    if (!_logon_enabled) return true;

    // Check error conditions if we're not in handler mode
    if (!handler_mode && _counts.decoder_unique > 3uz) impl().error();

    switch (bytes[0uz] & 0xF0u) {
      case 0b1111'0000u: return logonEnable(bytes);
      case 0b1101'0000u: return logonSelect(bytes);
      case 0b1110'0000u: return logonAssign(bytes);
    }

    return true;
  }

  /// Execute decoder control
  ///
  /// \retval true  Command executed
  /// \retval false Command not executed
  bool executeDecoderControl(std::span<uint8_t const>) const {
    /// \todo
    return false;
  }

  /// Execute consist control
  ///
  /// \param  bytes Raw bytes
  /// \retval true  Command executed
  /// \retval false Command not executed
  bool executeConsistControl(std::span<uint8_t const> bytes) {
    if (size(bytes) != 2uz + sizeof(_checksum)) return false;

    switch (bytes[0uz] & 0x0Fu) {
      case 0b0000'0010u: [[fallthrough]];
      case 0b0000'0011u:
        if (!(bytes[1uz] & ztl::mask<7u>) &&
            _counts.equal_packets == !DCC_STANDARD_COMPLIANCE + 1uz)
          cvWrite(19u - 1u,
                  static_cast<uint8_t>(bytes[0uz] << 7u | bytes[1uz]));
        break;
      default: return false;
    }

    return true;
  }

  /// Execute advanced operations
  ///
  /// \param  addr  Address
  /// \param  bytes Raw bytes
  /// \retval true  Command executed
  /// \retval false Command not executed
  bool executeAdvancedOperations(Address::value_type addr,
                                 std::span<uint8_t const> bytes) {
    switch (bytes[0uz]) {
      // Speed, direction and function
      case 0b0011'1100u:
        if (size(bytes) < 3uz + sizeof(_checksum)) return false;
        // F7-F0
        if (size(bytes) > 3uz)
          impl().function(
            addr, 0xFFu << 0u, static_cast<uint32_t>(bytes[2uz]) << 0u);
        // F15-F8
        if (size(bytes) > 4uz)
          impl().function(
            addr, 0xFFu << 8u, static_cast<uint32_t>(bytes[3uz]) << 8u);
        // F23-F16
        if (size(bytes) > 5uz)
          impl().function(
            addr, 0xFFu << 16u, static_cast<uint32_t>(bytes[4uz]) << 16u);
        // F31-F24
        if (size(bytes) > 6uz)
          impl().function(
            addr, 0xFFu << 0u, static_cast<uint32_t>(bytes[5uz]) << 24u);
        // Adjust length before fallthrough
        bytes = bytes.subspan<0uz, 2uz + sizeof(_checksum)>();
        [[fallthrough]];

      // 126 speed steps
      case 0b0011'1111u: {
        if (size(bytes) != 2uz + sizeof(_checksum)) return false;
        auto const dir{static_cast<bool>(bytes[1uz] & ztl::mask<7u>)};
        auto const speed{scale_speed<126>(decode_rggggggg(bytes[1uz]))};
        directionSpeed(addr, dir, speed);
        break;
      }

      // Special operating modes
      case 0b0011'1110u:
        if (size(bytes) != 2uz + sizeof(_checksum)) return false;
        _man = bytes[1uz] & ztl::mask<7u>;
        if constexpr (EastWest<T>) {
          if (bytes[1uz] & ztl::mask<6u>) // East
            impl().eastWestDirection(addr, East);
          else if (bytes[1uz] & ztl::mask<5u>) // West
            impl().eastWestDirection(addr, West);
          else // Neither
            impl().eastWestDirection(addr, std::nullopt);
        }
        break;

      // Analog function group
      case 0b0011'1101u:
        /// \todo
        break;
    }

    return true;
  }

  /// Execute speed and direction
  ///
  /// \param  addr  Address
  /// \param  bytes Raw bytes
  /// \retval true  Command executed
  /// \retval false Command not executed
  bool executeSpeedDirection(Address::value_type addr,
                             std::span<uint8_t const> bytes) {
    if (size(bytes) != 1uz + sizeof(_checksum)) return false;

    auto const dir{static_cast<bool>(bytes[0uz] & ztl::mask<5u>)};
    int32_t speed{};

    // 14 speed steps
    if (_f0_exception) {
      speed = scale_speed<14>(decode_rggggg(bytes[0uz], false));
      // F0
      if (addr) {
        constexpr auto mask{ztl::mask<0u>};
        auto const state{bytes[0uz] & ztl::mask<4u> ? ztl::mask<0u> : 0u};
        impl().function(addr, mask, state);
      }
    }
    // 28 speed steps
    else
      speed = scale_speed<28>(decode_rggggg(bytes[0uz], true));

    directionSpeed(addr, dir, speed);

    return true;
  }

  /// Execute function group
  ///
  /// This function receives the function output states for F0-F12 in three
  /// different sets. The command is one byte long.
  ///
  /// \param  addr  Address
  /// \param  bytes Raw bytes
  /// \retval true  Command executed
  /// \retval false Command not executed
  bool executeFunctionGroup(Address::value_type addr,
                            std::span<uint8_t const> bytes) {
    if (size(bytes) != 1uz + sizeof(_checksum)) return false;

    uint32_t mask{};
    uint32_t state{};

    switch (bytes[0uz] & 0xF0u) {
      case 0b1000'0000u: [[fallthrough]];
      case 0b1001'0000u:
        // x-x-x-F0-F4-F3-F2-F1
        mask = _f0_exception ? ztl::mask<4u, 3u, 2u, 1u>
                             : ztl::mask<4u, 3u, 2u, 1u, 0u>;
        state = (bytes[0uz] & 0xFu) << 1u | (bytes[0uz] & ztl::mask<4u>) >> 4u;
        break;

      case 0b1010'0000u:
        // x-x-x-x-F12-F11-F10-F9
        mask = ztl::mask<12u, 11u, 10u, 9u>;
        state = (bytes[0uz] & 0x0Fu) << 9u;
        break;

      case 0b1011'0000u:
        // x-x-x-x-F8-F7-F6-F5
        mask = ztl::mask<8u, 7u, 6u, 5u>;
        state = (bytes[0uz] & 0x0Fu) << 5u;
        break;
    }

    impl().function(addr, mask, state);

    return true;
  }

  /// Execute feature expansion
  ///
  /// This function is responsible to handle the binary state control, the
  /// rest of the function outputs (F13-28) and the time commands.
  ///
  /// \param  addr  Address
  /// \param  bytes Raw bytes
  /// \retval true  Command executed
  /// \retval false Command not executed
  bool executeFeatureExpansion(Address::value_type addr,
                               std::span<uint8_t const> bytes) {
    switch (bytes[0uz]) {
      // Binary state control instruction long form (3 bytes)
      case 0b1100'0000u:
        if (size(bytes) != 3uz + sizeof(_checksum)) return false;
        binaryState(addr,
                    (static_cast<uint32_t>(bytes[2uz]) << 7u) |
                      (bytes[1uz] & 0b0111'1111u),
                    bytes[1uz] & ztl::mask<7u>);
        break;

      // Binary state control instruction short form (2 bytes)
      case 0b1101'1101u:
        if (size(bytes) != 2uz + sizeof(_checksum)) return false;
        binaryState(
          addr, (bytes[1uz] & 0b0111'1111u), bytes[1uz] & ztl::mask<7u>);
        break;

      // Time (4 bytes)
      case 0b1100'0001u:
        if (size(bytes) != 4uz + sizeof(_checksum)) return false;
        time(bytes);
        break;

      // System time (3 bytes)
      case 0b1100'0010u:
        if (size(bytes) != 3uz + sizeof(_checksum)) return false;
        /// \todo
        break;

      // Command station properties (4 bytes)
      case 0b1100'0011u:
        if (size(bytes) != 4uz + sizeof(_checksum)) return false;
        /// \todo
        break;

      // F20-F19-F18-F17-F16-F15-F14-F13
      case 0b1101'1110u:
        if (size(bytes) != 2uz + sizeof(_checksum)) return false;
        impl().function(addr,
                        ztl::mask<20u, 19u, 18u, 17u, 16u, 15u, 14u, 13u>,
                        static_cast<uint32_t>(bytes[1uz]) << 13u);
        break;

      // F28-F27-F26-F25-F24-F23-F22-F21
      case 0b1101'1111u:
        if (size(bytes) != 2uz + sizeof(_checksum)) return false;
        impl().function(addr,
                        ztl::mask<28u, 27u, 26u, 25u, 24u, 23u, 22u, 21u>,
                        static_cast<uint32_t>(bytes[1uz]) << 21u);
        break;

      // F36-F35-F34-F33-F32-F31-F30-F29
      case 0b1101'1000u:
        if (size(bytes) != 2uz + sizeof(_checksum)) return false;
        break;

      // F44-F43-F42-F41-F40-F39-F38-F37
      case 0b1101'1001u:
        if (size(bytes) != 2uz + sizeof(_checksum)) return false;
        break;

      // F52-F51-F50-F49-F48-F47-F46-F45
      case 0b1101'1010u:
        if (size(bytes) != 2uz + sizeof(_checksum)) return false;
        break;

      // F60-F59-F58-F57-F56-F55-F54-F53
      case 0b1101'1011u:
        if (size(bytes) != 2uz + sizeof(_checksum)) return false;
        break;

      // F68-F67-F66-F65-F64-F63-F62-F61 (>F63 not supported)
      case 0b1101'1100u:
        if (size(bytes) != 2uz + sizeof(_checksum)) return false;
        break;
    }

    return true;
  }

  /// Execute CV access
  ///
  /// \param  addr  Address
  /// \param  bytes Raw bytes
  /// \retval true  Command executed
  /// \retval false Command not executed
  bool executeCvAccess(Address::value_type addr,
                       std::span<uint8_t const> bytes) {
    return bytes[0uz] & ztl::mask<4u> ? executeCvAccessShort(addr, bytes)
                                      : executeCvAccessLong(addr, bytes);
  }

  /// Execute CV access - long form
  ///
  /// \param  addr  Address
  /// \param  bytes Raw bytes
  /// \retval true  Command executed
  /// \retval false Command not executed
  bool executeCvAccessLong(Address::value_type addr,
                           std::span<uint8_t const> bytes) {
    if (size(bytes) < 3uz + sizeof(_checksum) ||
        size(bytes) > 8uz + sizeof(_checksum) ||
        (addr && addr == _addrs.consist))
      return false;

    // Type
    auto const kk{bytes[0uz] >> 2u & 0b11u};

    // POM
    if (size(bytes) == 3uz + sizeof(_checksum)) {
      // Store packet for app:pom
      if (_packets.pom != _deques.packet.front()) {
        _deques.pom.clear();
        _packets.pom = _deques.packet.front();
      }

      uint32_t const cv_addr{(bytes[0uz] & 0b11u) << 8u | bytes[1uz]};

      switch (kk) {
        // Reserved
        case 0b00u: break;

        // Verify byte
        case 0b01u: cvVerify(cv_addr, bytes[2uz]); break;

        // Write byte
        case 0b11u:
          // Write once
          if (_counts.equal_packets == 2uz) cvWrite(cv_addr, bytes[2uz]);
          // ...otherwise just verify
          else if (_counts.equal_packets > 2uz) cvVerify(cv_addr, bytes[2uz]);
          break;

        // Bit manipulation
        case 0b10u: {
          auto const pos{bytes[2uz] & 0b111u};
          auto const bit{static_cast<bool>(bytes[2uz] & ztl::mask<3u>)};
          if (!(bytes[2uz] & ztl::mask<4u>) || _counts.equal_packets > 2uz)
            cvVerify(cv_addr, bit, pos);
          else if (_counts.equal_packets == 2uz) cvWrite(cv_addr, bit, pos);
          break;
        }
      }
    }
    // XPOM
    else {
      auto const ss{static_cast<uint8_t>(bytes[0uz] & 0b11u)};

      // Already in deque
      if (std::ranges::any_of(_deques.xpom, [ss](auto const& dg) {
            return ((bidi::detail::decode_lut[dg[0uz]] >> 2u) & 0b11u) == ss;
          }))
        return true;

      uint32_t const cv_addr{static_cast<uint32_t>(bytes[1uz]) << 16u |
                             static_cast<uint32_t>(bytes[2uz]) << 8u |
                             static_cast<uint32_t>(bytes[3uz]) << 0u};

      switch (kk) {
        // Reserved
        case 0b00u: break;

        // Verify bytes
        case 0b01u:
          if (_counts.equal_packets == 1uz) xpomVerify(ss, cv_addr);
          break;

        // Write byte(s)
        case 0b11u:
          if (_counts.equal_packets == 2uz)
            xpomWrite(ss, cv_addr, bytes.subspan(4uz, size(bytes) - 4uz - 1uz));
          break;

        // Bit manipulation
        case 0b10u: {
          if (_counts.equal_packets == 2uz)
            xpomWrite(ss,
                      cv_addr,
                      static_cast<bool>(bytes[4uz] & ztl::mask<3u>),
                      bytes[4uz] & 0b111u);
          break;
        }
      }
    }

    return true;
  }

  /// Execute CV access - short form
  ///
  /// \param  addr  Address
  /// \param  bytes Raw bytes
  /// \retval true  Command executed
  /// \retval false Command not executed
  bool executeCvAccessShort(Address::value_type addr,
                            std::span<uint8_t const> bytes) {
    if (addr && addr == _addrs.consist) return false;

    // Type
    auto const kkkk{bytes[0uz] & 0x0Fu};

    switch (kkkk) {
      // Not available for use
      case 0b0000u: break;

      // Acceleration adjustment (CV23)
      case 0b0010u:
        if (size(bytes) != 2uz + sizeof(_checksum)) return false;
        else if (_counts.equal_packets == !DCC_STANDARD_COMPLIANCE + 1uz)
          cvWrite(23u - 1u, bytes[1uz]);
        break;

      // Deceleration adjustment (CV24)
      case 0b0011u:
        if (size(bytes) != 2uz + sizeof(_checksum)) return false;
        else if (_counts.equal_packets == !DCC_STANDARD_COMPLIANCE + 1uz)
          cvWrite(24u - 1u, bytes[1uz]);
        break;

      // Extended address (CV17 and CV18)
      case 0b0100u:
        if (size(bytes) != 3uz + sizeof(_checksum)) return false;
        else if (_counts.equal_packets == 2uz) {
          cvWrite(17u - 1u, static_cast<uint8_t>(0b1100'0000u | bytes[1uz]));
          cvWrite(18u - 1u, bytes[2uz]);
          cvWrite(29u - 1u, true, 5u);
        }
        break;

      // Index high and index low (CV31 and CV32)
      case 0b0101u:
        if (size(bytes) != 3uz + sizeof(_checksum)) return false;
        else if (_counts.equal_packets == 2uz) {
          cvWrite(31u - 1u, bytes[1uz]);
          cvWrite(32u - 1u, bytes[2uz]);
        }
        break;

      // Consist address (CV19 and CV20)
      case 0b0110u:
        if (size(bytes) != 3uz + sizeof(_checksum)) return false;
        else if (_counts.equal_packets == 2uz) {
          cvWrite(19u - 1u, bytes[1uz]);
          cvWrite(20u - 1u, bytes[2uz]);
        }
        break;

      // Reserved
      case 0b1001u: break;
    }

    return true;
  }

  /// CV byte and bit verify
  ///
  /// \param  cv_addr CV address
  /// \param  ts...   CV value or bit and bit position
  void cvVerify(uint32_t cv_addr, std::unsigned_integral auto... ts) {
    if (_cvs_locked) return;
    cvVerifyImpl(cv_addr, ts...);
  }

  /// CV byte verify
  ///
  /// \param  cv_addr CV address
  /// \param  byte    CV value
  void cvVerifyImpl(uint32_t cv_addr, uint8_t byte) {
    auto cb{[this, byte](uint8_t read_byte) {
      if (!serviceMode()) pom(read_byte);
      else if (byte == read_byte) impl().serviceAck();
    }};
    if (serviceMode() || !AsyncReadable<T>)
      std::invoke(cb, impl().readCv(cv_addr, byte));
    else if constexpr (AsyncReadable<T>) impl().readCv(cv_addr, byte, cb);
  }

  /// CV bit verify
  ///
  /// \param  cv_addr CV address
  /// \param  bit     CV bit
  /// \param  pos     CV bit position
  void cvVerifyImpl(uint32_t cv_addr, bool bit, uint32_t pos) {
    if ((impl().readCv(cv_addr, bit, pos) == bit) && serviceMode())
      impl().serviceAck();
  }

  /// CV byte and bit write, if necessary update init
  ///
  /// \param  cv_addr CV address
  /// \param  ts...   CV value or bit and bit position
  void cvWrite(uint32_t cv_addr, std::unsigned_integral auto... ts) {
    if (_cvs_locked && cv_addr != 15u - 1u) return;
    cvWriteImpl(cv_addr, ts...);
    if (std::ranges::contains(_init_cv_addrs, cv_addr)) {
      if (cv_addr == 1u - 1u) impl().writeCv(29u - 1u, false, 5u);
      init();
    }
  }

  /// CV byte write
  ///
  /// \param  cv_addr CV address
  /// \param  byte    CV value
  void cvWriteImpl(uint32_t cv_addr, uint8_t byte) {
    auto cb{[this, byte](uint8_t read_byte) {
      if (!serviceMode()) pom(read_byte);
      else if (byte == read_byte) impl().serviceAck();
    }};
    if (serviceMode() || !AsyncWritable<T>)
      std::invoke(cb, impl().writeCv(cv_addr, byte));
    else if constexpr (AsyncWritable<T>) impl().writeCv(cv_addr, byte, cb);
  }

  /// CV bit write
  ///
  /// \param  cv_addr CV address
  /// \param  bit     CV bit
  /// \param  pos     CV bit position
  void cvWriteImpl(uint32_t cv_addr, bool bit, uint32_t pos) {
    if ((impl().writeCv(cv_addr, bit, pos) == bit) && serviceMode())
      impl().serviceAck();
  }

  /// XPOM bytes verify
  ///
  /// \param  ss      Sequence number
  /// \param  cv_addr CV address
  void xpomVerify(uint8_t ss, uint32_t cv_addr) {
    if (_cvs_locked) return;
    xpomVerifyImpl(ss, cv_addr);
  }

  /// XPOM bytes verify
  ///
  /// \param  ss      Sequence number
  /// \param  cv_addr CV address
  void xpomVerifyImpl(uint8_t ss, uint32_t cv_addr) {
    std::array<uint8_t, 4uz> cvs;
    for (auto i{0uz}; i < size(cvs); ++i)
      cvs[i] = impl().readCv(static_cast<uint32_t>(cv_addr + i));
    xpom(ss, cvs);
  }

  /// XPOM bytes and bit write, if necessary update init
  ///
  /// \param  ss      Sequence number
  /// \param  cv_addr CV address
  void xpomWrite(uint8_t ss, uint32_t cv_addr, auto... ts) {
    if (_cvs_locked && cv_addr != 15u - 1u) return;
    xpomWriteImpl(ss, cv_addr, ts...);
    if (std::ranges::contains(_init_cv_addrs, cv_addr)) {
      if (cv_addr == 1u - 1u) impl().writeCv(29u - 1u, false, 5u);
      init();
    }
  }

  /// XPOM bytes write
  ///
  /// \param  ss      Sequence number
  /// \param  cv_addr CV address
  /// \param  bytes   CV values
  void
  xpomWriteImpl(uint8_t ss, uint32_t cv_addr, std::span<uint8_t const> bytes) {
    std::array<uint8_t, 4uz> cvs;
    for (auto i{0uz}; i < size(cvs); ++i)
      cvs[i] = i < size(bytes)
                 ? impl().writeCv(static_cast<uint32_t>(cv_addr + i), bytes[i])
                 : impl().readCv(static_cast<uint32_t>(cv_addr + i));
    xpom(ss, cvs);
  }

  /// XPOM bit write
  ///
  /// \param  ss      Sequence number
  /// \param  cv_addr CV address
  /// \param  bit     CV bit
  /// \param  pos     CV bit position
  void xpomWriteImpl(uint8_t ss, uint32_t cv_addr, bool bit, uint32_t pos) {
    impl().writeCv(cv_addr, bit, pos);
    xpomVerifyImpl(ss, cv_addr);
  }

  /// Register mode
  ///
  /// \param  bytes Raw bytes
  void registerMode(std::span<uint8_t const> bytes) {
    switch (auto const w{bytes[0uz] & 0b1000u}, reg{bytes[0uz] & 0b111u}; reg) {
      case 0u: [[fallthrough]];
      case 1u: [[fallthrough]];
      case 2u: [[fallthrough]];
      case 3u: {
        auto const i{_index_reg * 4u - (4u - reg)};
        w ? cvWrite(i, bytes[1uz]) : cvVerify(i, bytes[1uz]);
        break;
      }
      // CV29
      case 4u:
        w ? cvWrite(29u - 1u, bytes[1uz]) : cvVerify(29u - 1u, bytes[1uz]);
        break;
      // Index register
      case 5u:
        if (w) _index_reg = bytes[1uz];
        else if (_index_reg == bytes[1uz]) impl().serviceAck();
        break;
      // CV7
      case 6u: [[fallthrough]];
      // CV8
      case 7u: w ? cvWrite(reg, bytes[1uz]) : cvVerify(reg, bytes[1uz]); break;
    }
  }

  /// Time
  void time(std::span<uint8_t const>) const {
    /// \todo
  }

  /// Execute binary state
  ///
  /// \param  addr  Address
  /// \param  xf    Number of binary state
  /// \param  state Binary state
  void binaryState(Address::value_type, uint32_t xf, bool state) {
    switch (xf) {
      case 2u:
        if (!state) trackSearch();
        break;
      case 4u: break;
      case 5u: break;
    }
  }

  /// Set direction and speed
  ///
  /// \param  addr  Address
  /// \param  dir   Direction
  /// \param  speed Speed
  void directionSpeed(Address::value_type addr, bool dir, int32_t speed) {
    // Ignore direction on broadcast
    if (addr) {
      auto const reversed{addr == _addrs.primary ? _addrs.primary.reversed
                                                 : _addrs.primary.reversed ^
                                                     _addrs.consist.reversed};
      impl().direction(addr, reversed ? !dir : dir);
    }
    impl().speed(addr, speed);
  }

  /// Count own equal packets
  void countOwnEqualPackets() {
    if (_packets.last == _deques.packet.front()) ++_counts.equal_packets;
    else {
      _counts.equal_packets = 1uz;
      _packets.last = _deques.packet.front();
    }
  }

  /// Reset
  void reset() {
    _counts.bit = _byte = _checksum = 0u;
    _state = Preamble;
  }

  /// Enter or exit service mode
  ///
  /// \param  enter Enter service mode
  void serviceMode(bool enter) {
    // Disable other peripherals which might interfere
    if (enter) {
      impl().serviceModeHook(true);
      _mode = Service;
    } else {
      impl().serviceModeHook(false);
      _mode = Operations;
    }
  }

  /// Add to POM deque
  ///
  /// \param  byte  CV value
  void pom(uint8_t byte) {
    if (!_ch2_data_enabled) return;
    _deques.pom.clear();
    _deques.pom.push_back(bidi::make_app_pom_datagram(byte));
  }

  /// Add to XPOM deque
  ///
  /// \param  ss    Sequence number
  /// \param  bytes CV values
  void xpom(uint8_t ss, std::span<uint8_t const, 4uz> bytes) {
    if (!_ch2_data_enabled) return;
    _deques.xpom.push_back(bidi::make_app_xpom_datagram(ss, bytes));
  }

  /// Track search
  void trackSearch() {
    using std::literals::chrono_literals::operator""s;
    if (_backoffs.search || !empty(_deques.search)) return;
    auto const now{std::chrono::system_clock::now()};
    if (std::chrono::duration_cast<std::chrono::seconds>(now - _tps.init) >=
        30s)
      return;
    if (_tps.search == decltype(_tps.search){}) _tps.search = now;
    // Active address is logon
    if (_logon_assigned)
      _deques.search.push_back(bidi::make_app_search_datagram(
        _addrs.logon,
        0u,
        static_cast<uint8_t>(std::chrono::duration_cast<std::chrono::seconds>(
                               _tps.search - _tps.init)
                               .count())));
    // Active address is primary
    else if (!_addrs.consist)
      _deques.search.push_back(bidi::make_app_search_datagram(
        _addrs.primary,
        0u,
        static_cast<uint8_t>(std::chrono::duration_cast<std::chrono::seconds>(
                               _tps.search - _tps.init)
                               .count())));
    // Active address is consist
    else
      _deques.search.push_back(bidi::make_app_search_datagram(
        _addrs.consist,
        static_cast<uint8_t>((_addrs.consist.reversed ? 0x80u : 0u) |
                             (_addrs.consist & 0x7Fu)),
        static_cast<uint8_t>(std::chrono::duration_cast<std::chrono::seconds>(
                               _tps.search - _tps.init)
                               .count())));
  }

  /// Logon enable
  ///
  /// \param  bytes Raw bytes
  /// \retval true  Command executed
  /// \retval false Command not executed
  bool logonEnable(std::span<uint8_t const> bytes) {
    auto const cid{data2uint16(&bytes[1uz])};
    auto const sid{bytes[3uz]};

    // Already got selected and CID/SID didn't change
    if (_logon_selected && _ids.cs.back() == cid && _ids.session.back() == sid)
      return true;
    // ...otherwise clear selected
    else _logon_selected = false;

    // Store new CID and SID
    _ids.cs.back() = cid;
    _ids.session.back() = sid;

    // Skip logon if
    // - CIDs are equal and
    // - SIDs are equal if not yet logon assigned or
    // - difference between SIDs is <=1 if already logon assigned
    if ([[maybe_unused]] auto const skip{
          _ids.cs.back() == _ids.cs.front() &&
          static_cast<uint8_t>(_ids.session.back() - _ids.session.front()) <=
            _logon_assigned}) {
      _logon_selected = _logon_assigned = _logon_store = true;
      return true;
    }
    // ...otherwise force new logon
    else {
      _logon_assigned = false;
      _addrs.logon = {};
    }

    switch ([[maybe_unused]] auto const gg{
      static_cast<LogonGroup>(bytes[0uz] & 0b11u)}) {
      case LogonGroup::All: [[fallthrough]];              // All decoders
      case LogonGroup::Loco: break;                       // Loco decoders
      case LogonGroup::Acc: return true;                  // Accessory decoder
      case LogonGroup::Now: _backoffs.logon.now(); break; // No backoff
    }

    if (_backoffs.logon) return true;
    _deques.logon.clear();
    _deques.logon.push_back(bidi::make_app_decoder_unique_datagram(
      DCC_MANUFACTURER_ID, _ids.decoder));

    // Return false after 3 app:decoder_unique datagrams. This keeps the packet
    // in the deque to be picked up and executed in thread mode.
    return ++_counts.decoder_unique <= 3uz;
  }

  /// Logon select
  ///
  /// \param  bytes Raw bytes
  /// \retval true  Command executed
  /// \retval false Command not executed
  bool logonSelect(std::span<uint8_t const> bytes) {
    if (auto const did{bytes.subspan<2uz, sizeof(uint32_t)>()};
        _logon_assigned || !std::ranges::equal(did, _ids.decoder))
      return true;

    switch (bytes[6uz]) {
      // ShortInfo
      case 0b1111'1111u: break;
      // Read block
      case 0b1111'1110u: [[fallthrough]];
      // Write block
      case 0b1111'1100u: [[fallthrough]];
      // Set decoder internal status
      case 0b1111'1011u: [[fallthrough]];
      // Reserved
      default:
        _deques.logon.clear();
        _deques.logon.push_back({bidi::nak,
                                 bidi::nak,
                                 bidi::nak,
                                 bidi::nak,
                                 bidi::nak,
                                 bidi::nak,
                                 bidi::nak,
                                 bidi::nak});
        return true;
    }

    _logon_selected = true;
    std::array<uint8_t, 5uz> short_info{
      ztl::mask<7u>, // Special format & address
      0u,            // Address
      63u,           // Highest function
      ztl::mask<6u>, // XPOM
      0u};
    encode_logon_address(_addrs.primary, begin(short_info) + 1);
    _deques.logon.clear();
    _deques.logon.push_back(
      bidi::encode_datagram(bidi::make_datagram<bidi::Bits::_48>(
        static_cast<uint64_t>(short_info[0uz]) << 40u |
        static_cast<uint64_t>(short_info[1uz]) << 32u |
        static_cast<uint32_t>(short_info[2uz]) << 24u |
        static_cast<uint32_t>(short_info[3uz]) << 16u |
        static_cast<uint32_t>(short_info[4uz]) << 8u | crc8(short_info))));
    return true;
  }

  /// Logon assign
  ///
  /// \param  bytes Raw bytes
  /// \retval true  Command executed
  /// \retval false Command not executed
  bool logonAssign(std::span<uint8_t const> bytes) {
    if (auto const did{bytes.subspan<2uz, sizeof(uint32_t)>()};
        !std::ranges::equal(did, _ids.decoder))
      return true;

    auto const addr{decode_logon_address(cbegin(bytes) + 6)};

    // Don't accept assign
    if (addr.type != Address::BasicLoco && addr.type != Address::ExtendedLoco) {
      _deques.logon.clear();
      _deques.logon.push_back({bidi::nak,
                               bidi::nak,
                               bidi::nak,
                               bidi::nak,
                               bidi::nak,
                               bidi::nak,
                               bidi::nak,
                               bidi::nak});
      return true;
    }

    // Accept assign
    _logon_assigned = _logon_store = true;
    _addrs.consist = 0u;
    _addrs.logon = addr;
    // ... and permanent
    if (auto const bb{static_cast<LogonBindingBehavior>(bytes[6uz] >> 6u)};
        bb == LogonBindingBehavior::Permanent && addr)
      _addrs.primary = addr;
    _deques.logon.clear();
    _deques.logon.push_back(bidi::make_app_decoder_state_datagram(
      0xFFu,           // Change flags
      0u,              // Change count
      ztl::mask<7u,    // app:dyn ID7:27
                6u,    // app:dyn ID7:26
                4u,    // app:dyn ID7:7
                3u>,   // app:dyn ID7:0-1
      ztl::mask<6u,    // Special operating modes
                4u,    // CV access short
                3u,    // SDF
                2u,    // Binary state control long
                1u>)); // Binary state control short
    return true;
  }

  /// Add adr datagrams
  void adr() {
    if (!_ch1_addr_enabled || !empty(_deques.adr)) return;
    // Active address is logon
    else if (_logon_assigned) {
      _deques.adr.push_back(bidi::make_app_adr_high_datagram(_addrs.logon));
      _deques.adr.push_back(bidi::make_app_adr_low_datagram(_addrs.logon));
    }
    // Active address is primary
    else if (!_addrs.consist) {
      _deques.adr.push_back(bidi::make_app_adr_high_datagram(_addrs.primary));
      _deques.adr.push_back(bidi::make_app_adr_low_datagram(_addrs.primary));
    }
    // Active address is consist
    else if (_addrs.consist < 128u) {
      auto const cv19{static_cast<uint8_t>(
        (_addrs.consist.reversed ? 0x80u : 0u) | (_addrs.consist & 0x7Fu))};
      _deques.adr.push_back(
        bidi::make_app_adr_high_datagram(_addrs.consist, cv19));
      _deques.adr.push_back(
        bidi::make_app_adr_low_datagram(_addrs.consist, cv19));
    }
    /// Active address is extended consist...
    else {
      _deques.adr.push_back(bidi::make_app_adr_high_datagram(_addrs.consist));
      _deques.adr.push_back(bidi::make_app_adr_low_datagram(_addrs.consist));
    }
  }

  /// Add app:dyn datagrams
  ///
  /// \param  d DV (dynamic CV)
  /// \param  x Subindex
  void dyn(uint8_t d, uint8_t x) {
    if (!_ch2_data_enabled || full(_deques.dyn)) return;
    _deques.dyn.push_back(bidi::make_app_dyn_datagram(d, x));
  }

  /// Handle app:adr_low and app:adr_high datagrams
  void appAdr() {
    if (empty(_deques.adr)) return;
    _ch1 = _deques.adr.front();
    impl().transmitBiDi({cbegin(_ch1), size(_ch1)});
    _deques.adr.pop_front();
  }

  /// Handle app:pom
  void appPom() {
    // Deque contains data for this packet
    if (!empty(_deques.pom) &&
        (_packets.current == _packets.pom || _instr != Instruction::CvAccess)) {
      auto const& dg{_deques.pom.front()};
      std::copy(cbegin(dg), cend(dg), begin(_ch2));
      impl().transmitBiDi({cbegin(_ch2), size(dg)});
      _deques.pom.pop_front();
    }
    // Implicitly acknowledge all CV access commands
    else if (_ch2_data_enabled)
      impl().transmitBiDi({&bidi::acks[0uz].value(), size(bidi::acks)});
  }

  /// Handle app:dyn
  void appDyn() {
    if (empty(_deques.dyn)) return;
    auto first{begin(_ch2)};
    auto const last{cend(_ch2)};
    do {
      auto const& dg{_deques.dyn.front()};
      first = std::copy_n(cbegin(dg), size(dg), first);
      _deques.dyn.pop_front();
    } while (!empty(_deques.dyn) && last - first >= ssize(_deques.dyn.front()));
    impl().transmitBiDi({cbegin(_ch2), first});
  }

  /// Handle app:xpom
  void appXpom() {
    if (!empty(_deques.xpom)) {
      auto const& dg{_deques.xpom.front()};
      std::copy(cbegin(dg), cend(dg), begin(_ch2));
      impl().transmitBiDi({cbegin(_ch2), size(dg)});
      _deques.xpom.pop_front();
    }
    // Implicitly acknowledge all CV access commands
    else if (_ch2_data_enabled)
      impl().transmitBiDi({&bidi::acks[0uz].value(), size(bidi::acks)});
  }

  /// Handle app:search
  void appSearch() {
    if (empty(_deques.search)) return;
    auto const& dg{_deques.search.front()};
    std::ranges::copy(dg, begin(_ch2));
    impl().transmitBiDi({cbegin(_ch2), size(dg)});
    _deques.search.pop_front();
  }

  /// Handle app:logon
  void appLogon(uint32_t ch) {
    if (empty(_deques.logon)) return;
    if (auto const& dg{_deques.logon.front()}; ch == 1u) {
      std::copy(begin(dg), begin(dg) + 2, begin(_ch1));
      impl().transmitBiDi({cbegin(_ch1), size(_ch1)});
    } else {
      std::copy(begin(dg) + 2, end(dg), begin(_ch2));
      impl().transmitBiDi({cbegin(_ch2), size(_ch2)});
      _deques.logon.pop_front();
    }
  }

  /// Logon store
  ///
  /// RCN-218 requires us to answer extended packets directly in the following
  /// cutout. This is so time-critical that logon information can only be stored
  /// asynchronously...
  void logonStore() {
    if (!_logon_store) return;
    _logon_store = false;

    // Encode logon address
    std::array<uint8_t, 2uz> logon_addr_cvs{};
    encode_address(_addrs.logon, begin(logon_addr_cvs));

    // Logon assign is permanent
    if (_addrs.primary == _addrs.logon) {
      if (_addrs.logon.type == Address::BasicLoco) {
        impl().writeCv(1u - 1u, logon_addr_cvs[0uz]);
        impl().writeCv(29u - 1u, false, 5u);
      } else {
        impl().writeCv(17u - 1u, logon_addr_cvs[0uz]);
        impl().writeCv(18u - 1u, logon_addr_cvs[1uz]);
        impl().writeCv(29u - 1u, true, 5u);
      }
    }

    // Every assign clears eventually set consist address
    impl().writeCv(19u - 1u, 0u);
    impl().writeCv(20u - 1u, 0u);

    _ids.cs.front() = _ids.cs.back();
    impl().writeCv(DCC_RX_LOGON_CID_CV_ADDRESS + 0u,
                   static_cast<uint8_t>(_ids.cs.back() >> 8u));
    impl().writeCv(DCC_RX_LOGON_CID_CV_ADDRESS + 1u,
                   static_cast<uint8_t>(_ids.cs.back()));

    _ids.session.front() = _ids.session.back();
    impl().writeCv(DCC_RX_LOGON_SID_CV_ADDRESS, _ids.session.back());
    impl().writeCv(DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 0u, logon_addr_cvs[0uz]);
    impl().writeCv(DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 1u, logon_addr_cvs[1uz]);
  }

  /// Update quality of service every 200 packets (roughly every 2 seconds)
  void updateQos() {
    if (_counts.preamble < 200uz) return;
    _qos = static_cast<uint8_t>(
      100uz - (std::min(_counts.packet + 1uz, _counts.preamble) * 100uz) /
                _counts.preamble);
    _counts.packet = _counts.preamble = 0uz;
  }

  /// Update time points
  ///
  /// In case time between two packets is >=1s allow tip-off search again.
  void updateTimePoints() {
    using std::literals::chrono_literals::operator""s;
    auto const now{std::chrono::system_clock::now()};
    if (now - _tps.packet >= 1s) {
      _backoffs.search.now();
      _tps.search = decltype(_tps.search){};
      _tps.init = now;
    }
    _tps.packet = now;
  }

  // CVs where modification requires call of `init()`
  static constexpr std::array<uint8_t, 9uz> _init_cv_addrs{1u - 1u,
                                                           15u - 1u,
                                                           16u - 1u,
                                                           17u - 1u,
                                                           18u - 1u,
                                                           19u - 1u,
                                                           20u - 1u,
                                                           28u - 1u,
                                                           29u - 1u};

  // Counts
  struct {
    size_t one_bit{};        ///< Consecutive ones
    size_t bit{};            ///< Current bits
    size_t packet{};         ///< Successfully received packets
    size_t preamble{};       ///< Successfully received preambles
    size_t equal_packets{};  ///< Equal packets
    size_t decoder_unique{}; ///< app:decoder_unique transmissions
  } _counts{};

  // Time points
  struct {
    std::chrono::time_point<std::chrono::system_clock> init{};
    std::chrono::time_point<std::chrono::system_clock> packet{};
    std::chrono::time_point<std::chrono::system_clock> search{};
  } _tps{};

  // Deques
  struct {
    ztl::inplace_deque<Packet, DCC_RX_DEQUE_SIZE> packet{};
    ztl::inplace_deque<bidi::Datagram<bidi::datagram_size<bidi::Bits::_18>>,
                       DCC_RX_BIDI_DEQUE_SIZE>
      dyn{};
    ztl::inplace_deque<bidi::Datagram<bidi::datagram_size<bidi::Bits::_48>>,
                       1uz>
      logon{};
    ztl::inplace_deque<bidi::Datagram<bidi::datagram_size<bidi::Bits::_36>>,
                       1uz>
      search{};
    ztl::inplace_deque<bidi::Datagram<bidi::datagram_size<bidi::Bits::_12>>,
                       2uz>
      adr{};
    ztl::inplace_deque<bidi::Datagram<bidi::datagram_size<bidi::Bits::_12>>,
                       1uz>
      pom{};
    ztl::inplace_deque<bidi::Datagram<bidi::datagram_size<bidi::Bits::_36>>,
                       4uz>
      xpom{};
  } _deques{};

  // Packets
  struct {
    Packet current{}; ///< Current packet
    Packet last{};    ///< Last executed packet
    Packet pom{};     ///< Last executed POM packet
  } _packets{};

  // Backoffs
  struct {
    Backoff logon{};
    Backoff search{};
  } _backoffs{};

  // IDs
  struct {
    std::array<uint8_t, 4uz> decoder{}; ///< Decoder ID
    std::array<uint16_t, 2uz> cs{};     ///< Command station ID
    std::array<uint8_t, 2uz> session{}; ///< Session ID
  } _ids{};

  Addresses _addrs{};     ///< Addresses
  Instruction _instr{};   ///< Current instruction
  uint8_t _byte{};        ///< Current byte
  uint8_t _checksum{};    ///< On-the-fly calculated checksum
  uint8_t _index_reg{1u}; ///< Paged mode index register

  // Buffers
  bidi::Channel1 _ch1{};
  bidi::Channel2 _ch2{};

  uint8_t _qos{}; ///< Quality of service

  enum State : uint8_t { Preamble, Startbit, Data, Endbit } _state{};
  enum Mode : uint8_t { Operations, Service } _mode{};

  // Not bitfields as those are most likely mutated in interrupt context
  bool _is_halfbit{};
  bool _packet_end{};
  bool _ch1_addr_enabled{};
  bool _ch2_data_enabled{};
  bool _ch2_consist_enabled{};
  bool _logon_enabled{};
  bool _logon_selected{};
  bool _logon_assigned{};
  bool _logon_store{};

  bool _enabled : 1 {};
  bool _cvs_locked : 1 {};
  bool _f0_exception : 1 {};
  bool _man : 1 {};
  bool _block_dyn_deque : 1 {};
};

} // namespace dcc::rx
