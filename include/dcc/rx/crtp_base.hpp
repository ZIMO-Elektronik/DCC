// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive base
///
/// \file   dcc/rx/crtp_base.hpp
/// \author Vincent Hamp
/// \date   04/01/2022

#pragma once

#include <chrono>
#include <concepts>
#include <span>
#include <ztl/bits.hpp>
#include <ztl/inplace_deque.hpp>
#include "../addresses.hpp"
#include "../bidi/bundled_channels.hpp"
#include "../bidi/datagram.hpp"
#include "../bidi/direction_status_byte.hpp"
#include "../bidi/dyn.hpp"
#include "../bidi/kmh.hpp"
#include "../bidi/temperature.hpp"
#include "../bidi/track_voltage.hpp"
#include "../crc8.hpp"
#include "../direction.hpp"
#include "../instruction.hpp"
#include "../packet.hpp"
#include "../speed.hpp"
#include "../utility.hpp"
#include "async_readable.hpp"
#include "async_writable.hpp"
#include "backoff.hpp"
#include "decoder.hpp"
#include "east_west.hpp"
#include "high_current.hpp"
#include "timing.hpp"

namespace dcc::rx {

using namespace bidi;

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
    if (cv29 & ztl::make_mask(5u)) {
      std::array const cv17_cv18{impl().readCv(17u - 1u),
                                 impl().readCv(18u - 1u)};
      _addrs.primary = decode_address(cv17_cv18);
    } else {
      auto const cv1{impl().readCv(1u - 1u)};
      _addrs.primary = decode_address(&cv1);
    }
    _addrs.primary.reversed = cv29 & ztl::make_mask(0u);

    // Consist address
    auto const cv19{impl().readCv(19u - 1u)};
    auto const cv20{impl().readCv(20u - 1u)};
    auto const consist_addr{100u * (cv20 & 0b0111'1111u) +
                            (cv19 & 0b0111'1111u)};
    _addrs.consist = {static_cast<Address::value_type>(consist_addr),
                      Address::Long};
    _addrs.consist.reversed = cv19 & ztl::make_mask(7u);

    // Legacy exception for F0
    _f0_exception = !(cv29 & ztl::make_mask(1u));

    // Decoder lock
    auto const cv15{impl().readCv(15u - 1u)};
    auto const cv16{impl().readCv(16u - 1u)};
    _cvs_locked = cv15 != cv16 && cv15 && cv16;

    // BiDi
    auto const bidi_enabled{static_cast<bool>(cv29 & ztl::make_mask(3u))};
    auto const ch2_consist_enabled{
      static_cast<bool>(cv20 & ztl::make_mask(7u))};
    auto const cv28{impl().readCv(28u - 1u)};
    _ch1_addr_enabled = bidi_enabled && (cv28 & ztl::make_mask(0u));
    _ch2_data_enabled = bidi_enabled && (cv28 & ztl::make_mask(1u));
    _logon_enabled = bidi_enabled && (cv28 & ztl::make_mask(7u));
    _ch2_consist_enabled = bidi_enabled && ch2_consist_enabled;
    if constexpr (HighCurrent<T>)
      impl().highCurrentBiDi(cv28 & ztl::make_mask(6u));

    // IDs
    _did = {impl().readCv(250u - 1u),
            impl().readCv(251u - 1u),
            impl().readCv(252u - 1u),
            impl().readCv(253u - 1u)};
    _cids.front() = static_cast<decltype(_cids)::value_type>(
      (impl().readCv(65297u - 1u) << 8u) | impl().readCv(65298u - 1u));
    _session_ids.front() = impl().readCv(65299u - 1u);
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
    _packet_end = false;  // Whatever we got, its not packet end anymore

    auto const bit{time2bit(time)};
    if (bit == Invalid) return;

    // Alternate halfbit <-> bit
    if (_state > Startbit && (_is_halfbit = !_is_halfbit)) return;

    if (full(_deque)) return reset();  // TODO task full error counter?

    // Successfully received a bit
    switch (_state) {
      case Preamble:
        if (bit) ++_bit_count;
        else if (_bit_count < DCC_RX_MIN_PREAMBLE_BITS * 2uz) return reset();
        else _state = Startbit;
        break;

      case Startbit:
        _packet.clear();
        _bit_count = 0uz;
        _is_halfbit = false;
        ++_preamble_count;
        _state = Data;
        break;

      case Data:
        _byte = static_cast<uint8_t>((_byte << 1u) | bit);
        if (++_bit_count < 8uz) return;
        _packet.push_back(_byte);
        _checksum = static_cast<uint8_t>(_checksum ^ _byte);
        _bit_count = _byte = 0u;
        _state = Endbit;
        break;

      case Endbit:
        if (!bit) {
          _state = Data;
          return;
        }
        // Execute or push back valid packet
        else if (!_checksum && size(_packet) >= 3uz) {
          _packet_end = true;
          ++_packet_count;
          _addrs.received = decode_address(_packet);
          if (!executeHandlerMode()) _deque.push_back(_packet);
        }
        // Immediately clear received address and invalid packet
        else {
          _addrs.received = {};
          _packet.clear();
        }
        reset();
    }
  }

  /// Execute received commands
  ///
  /// \retval true  Command to own address
  /// \retval false Command to other address
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
  void biDiChannel1() {
    switch (_addrs.received.type) {
      case Address::Broadcast: [[fallthrough]];
      case Address::Short: [[fallthrough]];
      case Address::Long: appAdr(); break;
      case Address::AutomaticLogon: appLogon(1u); break;
      default: break;
    }
  }

  /// Start channel2 (36 bit payload)
  void biDiChannel2() {
    switch (_addrs.received.type) {
      case Address::Broadcast: appTos(); break;
      case Address::Short: [[fallthrough]];
      case Address::Long:
        if (_addrs.received ==
            (_logon_assigned ? _addrs.logon : _addrs.primary))
          decode_instruction(_packet) == Instruction::CvLong ? appPom()
                                                             : appDyn();
        else if (_addrs.received == _addrs.consist && _ch2_consist_enabled)
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
  /// \retval true  Command to AutomaticLogon address
  /// \retval false Command to other address
  bool executeHandlerMode() {
    if (_addrs.received.type != Address::AutomaticLogon) return false;
    if (size(_packet) <= (6uz + 1uz) || !crc8(_packet))
      executeAutomaticLogon(_addrs.received,
                            {cbegin(_packet) + 1, cend(_packet)});
    return true;
  }

  /// Execute in thread mode
  ///
  /// \retval true  Command to own address
  /// \retval false Command to other address
  bool executeThreadMode() {
    if (packetEnd() || empty(_deque)) return false;
    adr();               // Prepare address broadcasts for BiDi channel 1
    logonStore();        // Store logon information if necessary
    updateQos();         // Update quality of service
    updateTimepoints();  // Update timepoints for tip-off search
    auto const retval{serviceMode() ? executeService() : executeOperations()};
    _deque.pop_front();
    return retval;
  }

  /// Execute commands in operations mode
  ///
  /// \retval true  Command to own address or 0
  /// \retval false Command to other address
  bool executeOperations() {
    bool retval{};
    auto const& packet{_deque.front()};
    switch (auto const addr{decode_address(packet)}; addr.type) {
      case Address::IdleSystem:
        executeOperationsSystem({cbegin(packet) + 1, cend(packet)});
        break;
      case Address::Broadcast: [[fallthrough]];
      case Address::Short:
        retval =
          executeOperationsAddressed(addr, {cbegin(packet) + 1, cend(packet)});
        break;
      case Address::Long:
        retval =
          executeOperationsAddressed(addr, {cbegin(packet) + 2, cend(packet)});
        break;
      default: break;
    }
    return retval;
  }

  /// Execute commands in service mode
  ///
  /// \retval true
  bool executeService() {
    // Reset
    if (auto const& packet{_deque.front()}; !packet[0uz])
      ;
    // Exit
    else if ((packet[0uz] & 0xF0u) != 0b0111'0000u) serviceMode(false);
    // Register mode
    else if (size(packet) == 3uz) registerMode({cbegin(packet), cend(packet)});
    // CvLong
    else if (size(packet) == 4uz) cvLong(0u, {cbegin(packet), cend(packet)});
    return true;
  }

  /// Execute system commands
  void executeOperationsSystem(std::span<uint8_t const>) {
    /// \todo ?
  }

  /// Execute addressed commands
  ///
  /// \param  addr  Address
  /// \param  bytes Raw bytes
  /// \retval true  Command to own address or 0
  /// \retval false Command to other address
  bool executeOperationsAddressed(Address addr,
                                  std::span<uint8_t const> bytes) {
    // Address is broadcast
    if (!addr)
      ;
    // Address is primary or consist and logon ain't assigned
    else if ((addr == _addrs.primary || addr == _addrs.consist) &&
             !_addrs.logon)
      ;
    // Address is logon, pretend it's primary from here on
    else if (addr == _addrs.logon) addr = _addrs.primary;
    // Address is not of interest
    else return false;

    switch (decode_instruction(bytes)) {
      case Instruction::UnknownService:  // TODO
        break;
      case Instruction::DecoderControl:
        if (!addr && !bytes[0uz]) serviceMode(true);
        else decoderControl(bytes);
        break;
      case Instruction::ConsistControl: consistControl(bytes); break;
      case Instruction::AdvancedOperations:
        advancedOperations(addr, bytes);
        break;
      case Instruction::SpeedDirection: speedAndDirection(addr, bytes); break;
      case Instruction::FunctionGroup: functionGroup(addr, bytes); break;
      case Instruction::FeatureExpansion: featureExpansion(addr, bytes); break;
      case Instruction::CvLong: cvLong(addr, bytes); break;
      case Instruction::CvShort: cvShort(addr, bytes); break;
      default: break;
    }

    return true;
  }

  /// Execute automatic logon
  ///
  /// \param  addr  Address
  /// \param  bytes Raw bytes
  /// \retval true  Command to own address or 0
  /// \retval false Command to other address
  bool executeAutomaticLogon(Address addr, std::span<uint8_t const> bytes) {
    if (addr != 254u) return false;

    switch (bytes[0uz] & 0xF0u) {
      // SELECT
      case 0b1101'0000u: {
        auto const did{bytes.subspan<2uz, sizeof(uint32_t)>()};
        logonSelect(did);
        break;
      }

      // LOGON_ASSIGN
      case 0b1110'0000u: {
        auto const did{bytes.subspan<2uz, sizeof(uint32_t)>()};
        // Fucking stupid ZIMO version which overwrites primary address
        auto const overwrite_primary_address{(bytes[6uz] & 0b1100'0000u) !=
                                             0b1100'0000u};
        // Multi-function decoders (long address)
        if (auto const a13_8{bytes[6uz] & 0x3Fu}; a13_8 < 0x28u)
          logonAssign(
            did, decode_address(&bytes[6uz]), overwrite_primary_address);
        // Accessory decoder
        else if (a13_8 < 0x38u) break;
        // Multi-function decoders (short address)
        else if (a13_8 < 0x39u)
          logonAssign(
            did, decode_address(&bytes[7uz]), overwrite_primary_address);
        // Reserved
        else if (a13_8 < 0x3Fu) break;
        // FW update
        else if (a13_8 < 0x40u) break;
        break;
      }

      // LOGON_ENABLE
      case 0b1111'0000u: {
        auto const gg{static_cast<AddressGroup>(bytes[0uz] & 0b11u)};
        auto const cid{data2uint16(&bytes[1uz])};
        auto const session_id{bytes[3uz]};
        logonEnable(gg, cid, session_id);
        break;
      }
    }

    return true;
  }

  /// Execute decoder control
  /// \todo
  void decoderControl(std::span<uint8_t const>) const {}

  /// Execute consist control
  ///
  /// \param  bytes Raw bytes
  void consistControl(std::span<uint8_t const> bytes) {
    cvWrite(19u - 1u, static_cast<uint8_t>(bytes[0uz] << 7u | bytes[1uz]));
  }

  /// Execute advanced operations
  ///
  /// \param  addr  Address
  /// \param  bytes Raw bytes
  void advancedOperations(Address::value_type addr,
                          std::span<uint8_t const> bytes) {
    switch (bytes[0uz]) {
      // Speed, direction and function
      case 0b0011'1100u:
        // F7-F0
        if (size(bytes) > 3uz)
          impl().function(
            addr, 0xFFu << 0u, static_cast<uint32_t>(bytes[2uz] << 0u));
        // F15-F8
        if (size(bytes) > 4uz)
          impl().function(
            addr, 0xFFu << 8u, static_cast<uint32_t>(bytes[3uz] << 8u));
        // F23-F16
        if (size(bytes) > 5uz)
          impl().function(
            addr, 0xFFu << 16u, static_cast<uint32_t>(bytes[4uz] << 16u));
        // F31-F24
        if (size(bytes) > 6uz)
          impl().function(
            addr, 0xFFu << 0u, static_cast<uint32_t>(bytes[5uz] << 24u));
        [[fallthrough]];

      // 126 speed steps (plus 0)
      case 0b0011'1111u: {
        auto const dir{static_cast<bool>(bytes[1uz] & ztl::make_mask(7u))};
        // Stop
        if (!(bytes[1uz] & 0b0111'1111u)) directionSpeed(addr, dir, Stop);
        // Emergency stop
        else if (!(bytes[1uz] & 0b0111'1110u)) impl().speed(addr, EStop);
        else {
          auto const speed{scale_speed<126>((bytes[1uz] & 0b0111'1111) - 1)};
          directionSpeed(addr, dir, speed);
        }
        break;
      }

      // Special operating modes
      case 0b0011'1110u:
        _man = bytes[1uz] & ztl::make_mask(7u);
        if constexpr (EastWest<T>) {
          if (bytes[1uz] & ztl::make_mask(6u))  // East
            impl().eastWestDirection(addr, East);
          else if (ztl::make_mask(5u))  // West
            impl().eastWestDirection(addr, West);
          else  // Neither
            impl().eastWestDirection(addr, std::nullopt);
        }
        break;

      // Analog function group
      case 0b0011'1101u:
        /// \todo
        break;
    }
  }

  /// Execute speed and direction
  ///
  /// \param  addr  Address
  /// \param  bytes Raw bytes
  void speedAndDirection(Address::value_type addr,
                         std::span<uint8_t const> bytes) {
    auto const dir{static_cast<bool>(bytes[0uz] & ztl::make_mask(5u))};
    int32_t speed{};

    // Stop
    if (!(bytes[0uz] & 0b0000'1111u)) speed = Stop;
    // Emergency stop
    else if (!(bytes[0uz] & 0b0000'1110u)) return impl().speed(addr, EStop);
    else speed = static_cast<int32_t>(bytes[0uz] & 0b0000'1111u) - 1;

    // 14 speed steps and F0
    if (_f0_exception) {
      speed = scale_speed<14>(speed);
      auto const mask{ztl::make_mask(0u)};
      auto const state{bytes[0uz] & ztl::make_mask(4u) ? ztl::make_mask(0u)
                                                       : 0u};
      impl().function(addr, mask, state);
    }
    // 28 speed steps with intermediate
    else {
      speed <<= 1u;
      if (speed && !(bytes[0uz] & ztl::make_mask(4u))) --speed;
      speed = scale_speed<28>(speed);
    }

    directionSpeed(addr, dir, speed);
  }

  /// Execute function group
  ///
  /// This function receives the function output states for F0-F12 in three
  /// different sets. The command is one byte long.
  ///
  /// \param  addr  Address
  /// \param  bytes Raw bytes
  void functionGroup(Address::value_type addr, std::span<uint8_t const> bytes) {
    uint32_t mask{};
    uint32_t state{};

    switch (bytes[0uz] & 0xF0u) {
      case 0b1000'0000u: [[fallthrough]];
      case 0b1001'0000u:
        // x-x-x-F0-F4-F3-F2-F1
        mask = _f0_exception ? ztl::make_mask(4u, 3u, 2u, 1u)
                             : ztl::make_mask(4u, 3u, 2u, 1u, 0u);
        state =
          (bytes[0uz] & 0xFu) << 1u | (bytes[0uz] & ztl::make_mask(4u)) >> 4u;
        break;

      case 0b1011'0000u:
        // x-x-x-x-F8-F7-F6-F5
        mask = ztl::make_mask(8u, 7u, 6u, 5u);
        state = (bytes[0uz] & 0x0Fu) << 5u;
        break;

      case 0b1010'0000u:
        // x-x-x-x-F12-F11-F10-F9
        mask = ztl::make_mask(12u, 11u, 10u, 9u);
        state = (bytes[0uz] & 0x0Fu) << 9u;
        break;
    }

    impl().function(addr, mask, state);
  }

  /// Execute feature expansion
  ///
  /// This function is responsible to handle the binary state control, the
  /// rest of the function outputs (F13-28) and the time commands.
  ///
  /// \param  addr  Address
  /// \param  bytes Raw bytes
  void featureExpansion(Address::value_type addr,
                        std::span<uint8_t const> bytes) {
    switch (bytes[0uz]) {
      // Binary state control instruction long form (3 bytes)
      case 0b1100'0000u:
        binaryState((static_cast<uint32_t>(bytes[2uz]) << 7u) |
                      (bytes[1uz] & 0b0111'1111u),
                    bytes[1uz] & ztl::make_mask(7u));
        break;

      // Binary state control instruction short form (2 bytes)
      case 0b1101'1101u:
        binaryState((bytes[1uz] & 0b0111'1111u),
                    bytes[1uz] & ztl::make_mask(7u));
        break;

      // Time (4 bytes)
      case 0b1100'0001u: time(bytes); break;

      // System time (3 bytes)
      case 0b1100'0010u:
        /// \todo
        break;

      // Command station properties (4 bytes)
      case 0b1100'0011u:
        /// \todo
        break;

      // F20-F19-F18-F17-F16-F15-F14-F13
      case 0b1101'1110u:
        impl().function(addr,
                        ztl::make_mask(20u, 19u, 18u, 17u, 16u, 15u, 14u, 13u),
                        static_cast<uint32_t>(bytes[1uz] << 13u));
        break;

      // F28-F27-F26-F25-F24-F23-F22-F21
      case 0b1101'1111u:
        impl().function(addr,
                        ztl::make_mask(28u, 27u, 26u, 25u, 24u, 23u, 22u, 21u),
                        static_cast<uint32_t>(bytes[1uz] << 21u));
        break;

      // F36-F35-F34-F33-F32-F31-F30-F29
      case 0b1101'1000u: break;

      // F44-F43-F42-F41-F40-F39-F38-F37
      case 0b1101'1001u: break;

      // F52-F51-F50-F49-F48-F47-F46-F45
      case 0b1101'1010u: break;

      // F60-F59-F58-F57-F56-F55-F54-F53
      case 0b1101'1011u: break;

      // F68-F67-F66-F65-F64-F63-F62-F61 (>F63 not supported)
      case 0b1101'1100u: break;
    }
  }

  /// Execute CV access - long form
  ///
  ///
  /// \param  addr  Address
  /// \param  bytes Raw bytes
  void cvLong(Address::value_type addr, std::span<uint8_t const> bytes) {
    if (addr && addr == _addrs.consist) return;

    countOwnEqualCvPackets();

    // Ignore all but the second packet while in service mode
    if (serviceMode() && _own_equal_cv_packets_count != 2uz) return;

    switch (uint32_t const cv_addr{(bytes[0uz] & 0b11u) << 8u | bytes[1uz]};
            static_cast<uint32_t>(bytes[0uz]) >> 2u & 0b11u) {
      // Reserved
      case 0b00u: break;

      // Verify byte
      case 0b01u: cvVerify(cv_addr, bytes[2uz]); break;

      // Write byte
      case 0b11u:
        if (_own_equal_cv_packets_count < 2uz) return;
        else if (_own_equal_cv_packets_count == 2uz)
          cvWrite(cv_addr, bytes[2uz]);
        else cvVerify(cv_addr, bytes[2uz]);
        break;

      // Bit manipulation
      case 0b10u: {
        auto const pos{bytes[2uz] & 0b111u};
        auto const bit{static_cast<bool>(bytes[2uz] & ztl::make_mask(3u))};
        if (!(bytes[2uz] & ztl::make_mask(4u))) cvVerify(cv_addr, bit, pos);
        else if (_own_equal_cv_packets_count == 2uz) cvWrite(cv_addr, bit, pos);
        break;
      }
    }
  }

  /// Execute CV access - short form
  ///
  /// \param  addr  Address
  /// \param  bytes Raw bytes
  void cvShort(Address::value_type addr, std::span<uint8_t const> bytes) {
    if (addr && addr == _addrs.consist) return;

    countOwnEqualCvPackets();

    switch (bytes[0uz] & 0x0Fu) {
      // Not available for use
      case 0b0000u: break;

      // Acceleration adjustment (CV23)
      case 0b0010u:
        cvWrite(23u - 1u, bytes[1uz]);  // Fine if written at once
        break;

      // Deceleration adjustment (CV24)
      case 0b0011u:
        cvWrite(24u - 1u, bytes[1uz]);  // Fine if written at once
        break;

      // Extended address 0 and 1 (CV17 and CV18)
      case 0b0100u:
        if (_own_equal_cv_packets_count != 2uz) return;
        cvWrite(17u - 1u, static_cast<uint8_t>(0b1100'0000u | bytes[1uz]));
        cvWrite(18u - 1u, bytes[2uz]);
        cvWrite(29u - 1u, true, 5u);
        break;

      // Index high and index low (CV31 and CV32)
      case 0b0101u:
        if (_own_equal_cv_packets_count != 2uz) return;
        cvWrite(31u - 1u, bytes[1uz]);
        cvWrite(32u - 1u, bytes[2uz]);
        break;

      case 0b1001u: break;
    }
  }

  /// CV byte and bit verify
  ///
  /// \param  cv_addr CV address
  /// \param  ts...   CV value or bit and bit position
  void cvVerify(uint32_t cv_addr, auto... ts) {
    if (_cvs_locked) return;
    cvVerifyImpl(cv_addr, ts...);
  }

  /// CV byte verify
  ///
  /// \param  cv_addr CV address
  /// \param  byte    CV value
  void cvVerifyImpl(uint32_t cv_addr, uint8_t byte) {
    auto cb{[this, byte](uint8_t red_byte) {
      if (!serviceMode()) pom(red_byte);
      else if (byte == red_byte) impl().serviceAck();
    }};
    if constexpr (AsyncReadable<T>) impl().readCv(cv_addr, byte, cb);
    else std::invoke(cb, impl().readCv(cv_addr, byte));
  }

  /// CV bit verify
  ///
  /// \param  cv_addr CV address
  /// \param  bit     CV bit
  /// \param  pos     CV bit position
  void cvVerifyImpl(uint32_t cv_addr, bool bit, uint32_t pos) {
    auto cb{[this, bit](bool red_bit) {
      if (!serviceMode()) pom(red_bit);
      else if (bit == red_bit) impl().serviceAck();
    }};
    if constexpr (AsyncReadable<T>) impl().readCv(cv_addr, bit, pos, cb);
    else std::invoke(cb, impl().readCv(cv_addr, bit, pos));
  }

  /// CV byte and bit write, if necessary update init
  ///
  /// \param  cv_addr CV address
  /// \param  ts...   CV value or bit and bit position
  void cvWrite(uint32_t cv_addr, auto... ts) {
    if (_cvs_locked && cv_addr != 15u - 1u) return;
    cvWriteImpl(cv_addr, ts...);
    static constexpr std::array<uint8_t, 9uz> cvs{1u - 1u,
                                                  15u - 1u,
                                                  16u - 1u,
                                                  17u - 1u,
                                                  18u - 1u,
                                                  19u - 1u,
                                                  20u - 1u,
                                                  28u - 1u,
                                                  29u - 1u};
    if (std::ranges::any_of(cvs, [cv_addr](uint8_t a) { return a == cv_addr; }))
      init();
  }

  /// CV byte write
  ///
  /// \param  cv_addr CV address
  /// \param  byte    CV value
  void cvWriteImpl(uint32_t cv_addr, uint8_t byte) {
    auto cb{[this, byte](uint8_t red_byte) {
      if (!serviceMode()) pom(red_byte);
      else if (byte == red_byte) impl().serviceAck();
    }};
    if constexpr (AsyncWritable<T>) impl().writeCv(cv_addr, byte, cb);
    else std::invoke(cb, impl().writeCv(cv_addr, byte));
  }

  /// CV bit write
  ///
  /// \param  cv_addr CV address
  /// \param  bit     CV bit
  /// \param  pos     CV bit position
  void cvWriteImpl(uint32_t cv_addr, bool bit, uint32_t pos) {
    auto cb{[this, bit](bool red_bit) {
      if (!serviceMode()) pom(red_bit);
      else if (red_bit == bit) impl().serviceAck();
    }};
    if constexpr (AsyncWritable<T>) impl().writeCv(cv_addr, bit, pos, cb);
    else std::invoke(cb, impl().writeCv(cv_addr, bit, pos));
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
  /// \todo
  void time(std::span<uint8_t const>) const {}

  /// Execute binary state
  ///
  /// \param  xf    Number of binary state
  /// \param  state Binary state
  void binaryState(uint32_t xf, bool state) {
    switch (xf) {
      case 2u:
        if (!state) tipOffSearch();
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
    auto const reversed{addr == _addrs.primary ? _addrs.primary.reversed
                                               : _addrs.consist.reversed};
    impl().direction(addr, reversed ? !dir : dir);
    impl().speed(addr, speed);
  }

  /// Count own equal CV packets
  void countOwnEqualCvPackets() {
    if (_last_own_cv_packet == _deque.front()) ++_own_equal_cv_packets_count;
    else {
      _own_equal_cv_packets_count = 1uz;
      _last_own_cv_packet = _deque.front();
      _pom_deque.clear();
    }
  }

  /// Reset
  void reset() {
    _bit_count = _byte = _checksum = 0u;
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

  /// Add to pom deque
  ///
  /// \param  value CV value
  void pom(uint8_t value) {
    if (!_ch2_data_enabled) return;
    _pom_deque.clear();
    _pom_deque.push_back(encode_datagram(make_datagram<Bits::_12>(0u, value)));
  }

  /// Tip-off search
  void tipOffSearch() {
    using std::literals::chrono_literals::operator""s;
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
        _addrs.logon = decode_address(cv65300_65301);
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
    _logon_deque.push_back(encode_datagram(make_datagram<Bits::_48>(
      15u,
      static_cast<uint64_t>(DCC_MANUFACTURER_ID) << 32u |
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
    _addrs.consist = 0u;
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

  /// Add adr datagrams
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
      /// \todo
    }
  }

  /// Add generic dyn datagram
  ///
  /// \param  d Generic dyn datagram
  template<std::derived_from<Dyn> Dyns>
  void dyn(Dyns&& d) {
    if constexpr (std::same_as<Dyns, Dyn>) dyn(d.d, d.x);
    else if constexpr (std::same_as<Dyns, Kmh>) {
      auto const tmp{d < 512 ? (d < 256 ? d : d - 256) : 255};
      dyn(static_cast<uint8_t>(tmp), d < 256 ? 0u : 1u);
    } else if constexpr (std::same_as<Dyns, Temperature>) {
      auto const tmp{ztl::lerp<Dyn::value_type>(d.d, -50, 205, 0, 255)};
      dyn(static_cast<uint8_t>(tmp), 26u);
    } else if constexpr (std::same_as<Dyns, DirectionStatusByte>)
      dyn(static_cast<uint8_t>(d), 27u);
    else if constexpr (std::same_as<Dyns, TrackVoltage>) {
      auto const tmp{std::max<Dyn::value_type>(0, d - 5000)};
      dyn(static_cast<uint8_t>(tmp / 100), 46u);
    }
  }

  /// Add app:dyn datagrams
  ///
  /// \param  d DV (dynamic CV)
  /// \param  x Subindex
  void dyn(uint8_t d, uint8_t x) {
    if (!_ch2_data_enabled || full(_dyn_deque)) return;
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
    if (!_ch2_data_enabled) return;
    // Implicitly acknowledge all CV access commands
    else if (_packet != _last_own_cv_packet)
      impl().transmitBiDi({cbegin(acks), sizeof(acks[0uz])});
    else if (!empty(_pom_deque)) {
      auto const& datagram{_pom_deque.front()};
      std::copy(cbegin(datagram), cend(datagram), begin(_ch2));
      impl().transmitBiDi({cbegin(_ch2), size(datagram)});
      _pom_deque.pop_front();
    }
  }

  /// Handle app:dyn
  void appDyn() {
    if (empty(_dyn_deque)) return;
    auto first{begin(_ch2)};
    auto const last{cend(_ch2)};
    do {
      auto const& datagram{_dyn_deque.front()};
      first = std::copy_n(cbegin(datagram), size(datagram), first);
      _dyn_deque.pop_front();
    } while (!empty(_dyn_deque) && last - first >= ssize(_dyn_deque.front()));
    impl().transmitBiDi({cbegin(_ch2), first});
  }

  /// Handle app:tos
  void appTos() {
    if (empty(_tos_deque)) return;
    auto const& datagram{_tos_deque.front()};
    std::ranges::copy(datagram, begin(_ch2));
    impl().transmitBiDi({cbegin(_ch2), size(datagram)});
    _tos_deque.pop_front();
  }

  /// Handle app::logon
  void appLogon(uint32_t ch) {
    if (empty(_logon_deque)) return;
    if (auto const& datagram{_logon_deque.front()}; ch == 1u) {
      std::copy(begin(datagram), begin(datagram) + 2, begin(_ch1));
      impl().transmitBiDi({cbegin(_ch1), size(_ch1)});
    } else {
      std::copy(begin(datagram) + 2, end(datagram), begin(_ch2));
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

    // Every assign clears eventually set consist address
    impl().writeCv(19u - 1u, 0u);
    impl().writeCv(20u - 1u, 0u);

    _cids.front() = _cids.back();
    impl().writeCv(65297u - 1u, static_cast<uint8_t>(_cids.back() >> 8u));
    impl().writeCv(65298u - 1u, static_cast<uint8_t>(_cids.back()));

    _session_ids.front() = _session_ids.back();
    impl().writeCv(65299u - 1u, _session_ids.back());
    impl().writeCv(65300u - 1u, static_cast<uint8_t>(_addrs.logon >> 8u));
    impl().writeCv(65301u - 1u, static_cast<uint8_t>(_addrs.logon));
  }

  /// Update quality of service (roughly every second)
  ///
  /// \return Quality of service
  void updateQos() {
    if (_preamble_count < 100u) return;
    _qos =
      static_cast<uint8_t>(100u - (_packet_count * 100u) / _preamble_count);
    _packet_count = _preamble_count = 0u;
  }

  /// Update time points
  ///
  /// In case time between two packets is >=2s allow tip-off search again.
  void updateTimepoints() {
    using std::literals::chrono_literals::operator""s;
    auto const packet_tp{std::chrono::system_clock::now()};
    if (packet_tp - _last_packet_tp >= 2s) {
      _tos_backoff.now();
      _tos_tp = packet_tp;
    }
    _last_packet_tp = packet_tp;
  }

  // Deques
  ztl::inplace_deque<Packet, DCC_RX_DEQUE_SIZE> _deque{};
  ztl::inplace_deque<Datagram<datagram_size<Bits::_18>>, DCC_RX_BIDI_DEQUE_SIZE>
    _dyn_deque{};
  ztl::inplace_deque<Datagram<datagram_size<Bits::_48>>, 1uz> _logon_deque{};
  ztl::inplace_deque<Datagram<datagram_size<Bits::_36>>, 1uz> _tos_deque{};
  ztl::inplace_deque<Datagram<datagram_size<Bits::_12>>, 2uz> _adr_deque{};
  ztl::inplace_deque<Datagram<datagram_size<Bits::_12>>, 2uz> _pom_deque{};

  Packet _packet{};              ///< Current packet
  Packet _last_own_cv_packet{};  ///< Last CV packet for own address

  Addresses _addrs{};

  size_t _bit_count{};
  size_t _packet_count{};
  size_t _preamble_count{};
  size_t _own_equal_cv_packets_count{};
  uint8_t _byte{};
  uint8_t _checksum{};     ///< On-the-fly calculated checksum
  uint8_t _index_reg{1u};  ///< Paged mode index register

  enum State : uint8_t { Preamble, Startbit, Data, Endbit } _state{};
  enum Mode : uint8_t { Operations, Service } _mode{};

  // Timepoints
  std::chrono::time_point<std::chrono::system_clock> _last_packet_tp{};
  std::chrono::time_point<std::chrono::system_clock> _tos_tp{};

  std::array<uint8_t, 4uz> _did{};

  // Buffers
  Channel1 _ch1{};
  Channel2 _ch2{};

  Backoff _logon_backoff{};
  Backoff _tos_backoff{};

  std::array<uint16_t, 2uz> _cids{};        ///< Central ID
  std::array<uint8_t, 2uz> _session_ids{};  ///< Session ID

  uint8_t _qos{};  ///< Quality of service

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

}  // namespace dcc::rx