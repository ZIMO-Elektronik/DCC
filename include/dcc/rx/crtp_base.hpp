// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive base
///
/// \file   dcc/rx/crtp_base.hpp
/// \author Vincent Hamp
/// \date   04/01/2022

#pragma once

#include <span>
#include <ztl/bits.hpp>
#include "../addresses.hpp"
#include "../bit.hpp"
#include "../crc8.hpp"
#include "../instruction.hpp"
#include "../packet.hpp"
#include "../utility.hpp"
#include "async_readable.hpp"
#include "async_writable.hpp"
#include "bidi/crtp_base.hpp"
#include "decoder.hpp"
#include "east_west_man.hpp"
#include "mdu_entry.hpp"

namespace dcc::rx {

/// CRTP base for receiving DCC
///
/// \tparam T Type to downcast to
template<typename T>
struct CrtpBase : bidi::CrtpBase<T> {
  friend T;
  using BiDi = bidi::CrtpBase<T>;

  /// Initialize
  void init() { config(); }

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
    if (_state != State::Preamble && (_is_halfbit = !_is_halfbit)) return;

    if (full(_deque)) return reset();  // TODO task full error counter?

    // Successfully received a bit -> enter state machine
    switch (_state) {
      case State::Preamble:
        // Is bit a preamble "one"
        if (bit) ++_bit_count;
        // In case it's not, and we have less than enough preamble bits
        else if (_bit_count < DCC_RX_PREAMBLE_BITS * 2uz) return reset();
        // In case it's not and we have enough preamble bits but this is only
        // the first half of the packet startbit
        else if (!_byte_count) ++_byte_count;
        else {
          _bit_count = _byte_count = 0uz;
          _is_halfbit = false;
          ++_preamble_count;  // Count received preambles
          flush();            // Flush since new command is replacing old one
          _state = State::Data;
        }
        break;

      case State::Data: {
        auto data{end(_deque)->data()};
        data[_byte_count] =
          static_cast<uint8_t>((data[_byte_count] << 1u) | bit);
        if (++_bit_count < 8uz) return;
        _bit_count = 0uz;
        _checksum = static_cast<uint8_t>(_checksum ^ data[_byte_count++]);
        _state = State::Endbit;
        break;
      }

      case State::Endbit:
        if (!bit) _state = State::Data;
        else if (_checksum) return reset();
        else {
          end(_deque)->resize(static_cast<Packet::size_type>(_byte_count));
          _bit_count = _byte_count = _checksum = 0u;
          ++_packet_count;  // Count received packets
          _deque.push_back();
          _packet_end = true;
          executeHandlerMode();
          return reset();
        }
        break;
    }
  }

  /// Execute received commands
  ///
  ///
  /// \tparam Dyns... Types of dyn messages
  /// \param  dyns... Messages
  /// \return true    Command to own address
  /// \return false   Command to other address
  template<std::derived_from<bidi::Dyn>... Dyns>
  bool execute(Dyns&&... dyns)
    requires((sizeof...(Dyns) <
              DCC_RX_BIDI_DEQUE_SIZE))  // TODO remove double braces, currently
                                        // fucks with VSCode highlighting
  {
    BiDi::execute(std::forward<Dyns>(dyns)...);
    return executeThreadMode();
  }

  /// Service mode
  ///
  /// \return true  Service mode active
  /// \return false Operations mode active
  bool serviceMode() const { return _mode == Mode::Service; }

  /// MAN function
  ///
  /// \return true  MAN function active
  /// \return false MAN function inactive
  bool man() const { return _man; }

  /// Check if last received bit was a packet end
  ///
  /// \return true  Last received bit was packet end
  /// \return false Last received bit wasn't packet end
  bool packetEnd() const { return _packet_end; }

  /// Start channel1 (12 bit payload)
  void cutoutChannel1() { BiDi::cutoutChannel1(); }

  /// Start channel2 (36 bit payload)
  void cutoutChannel2() { BiDi::cutoutChannel2(); }

protected:
  using BiDi::_addrs;
  using BiDi::impl;

private:
  constexpr CrtpBase() = default;

  /// Configure
  void config() {
    auto const cv29{impl().readCv(29u - 1u)};
    if (cv29 & ztl::make_mask(5u)) {
      std::array const cv17_18{impl().readCv(17u - 1u),
                               impl().readCv(18u - 1u)};
      _addrs.primary = decode_address(cbegin(cv17_18));
    } else {
      auto const cv1{impl().readCv(1u - 1u)};
      _addrs.primary = decode_address(&cv1);
    }
    auto const cv19{impl().readCv(19u - 1u)};
    auto const cv20{impl().readCv(20u - 1u)};
    auto const addr{100u * (cv20 & 0b0111'1111u) + (cv19 & 0b0111'1111u)};
    _addrs.consist = {static_cast<Address::value_type>(addr), Address::Long};
    _f0_exception = !(cv29 & ztl::make_mask(1u));
    auto const cv15{impl().readCv(15u - 1u)};
    auto const cv16{impl().readCv(16u - 1u)};
    _cvs_locked = cv15 != cv16 && cv15 && cv16;
    BiDi::config(cv29 & ztl::make_mask(3u), cv20 & ztl::make_mask(7u));
  }

  /// Update configuration when writing certain CVs
  ///
  /// \param  addr  CV address
  void updateConfig(uint32_t addr) {
    static constexpr std::array<uint8_t, 9uz> cvs{1u - 1u,
                                                  15u - 1u,
                                                  16u - 1u,
                                                  17u - 1u,
                                                  18u - 1u,
                                                  19u - 1u,
                                                  20u - 1u,
                                                  28u - 1u,
                                                  29u - 1u};
    if (std::ranges::any_of(cvs, [addr](uint8_t a) { return a == addr; }))
      config();
  }

  /// Execute in thread mode
  ///
  /// \return true  Command to own address
  /// \return false Command to other address
  bool executeThreadMode() {
    if (empty(_deque)) return false;
    auto const retval{_mode == Mode::Operations ? executeOperations()
                                                : executeService()};
    _deque.pop_front();
    BiDi::executeThreadMode();
    return retval;
  }

  /// Execute in handler mode (interrupt context)
  void executeHandlerMode() {
    auto const& packet{_deque.front()};
    auto const addr{decode_address(cbegin(packet))};
    _addrs.received = addr;
    if (addr.type != Address::AutomaticLogon) return;
    if (size(packet) <= 7uz || !crc8(packet))
      executeAutomaticLogon(addr, {cbegin(packet) + 1, cend(packet)});
    _deque.pop_front();
  }

  /// Execute commands in operations mode
  ///
  /// \return true  Command to own address or 0
  /// \return false Command to other address
  bool executeOperations() {
    bool retval{};
    auto const& packet{_deque.front()};
    switch (auto const addr{decode_address(cbegin(packet))}; addr.type) {
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
    qos();
    return retval;
  }

  /// Execute commands in service mode
  ///
  /// \return true
  bool executeService() {
    countOwnEqualPackets();
    // Reset
    if (auto const& packet{_deque.front()}; !packet[0uz])
      ;
    // Exit
    else if ((packet[0uz] & 0xF0u) != 0b0111'0000u) serviceMode(false);
    // Register mode
    else if (size(packet) == 3uz) registerMode({cbegin(packet), cend(packet)});
    // CvLong
    else if (size(packet) == 4uz && _own_equal_packets_count == 2uz)
      cvLong({cbegin(packet), cend(packet)});
    return true;
  }

  /// Execute system commands
  ///
  /// \param  chunk Raw data
  void executeOperationsSystem(std::span<uint8_t const> chunk) {
    switch (chunk[0uz]) {
      case 0x01u: _zimo = chunk[1uz] == zimo_id; break;
      case 0x02u:
        // Decoder search
        break;
      case 0x03u:
        // Direct access?
        break;
      case 0x04u:
        // Set address?
        break;
    }
  }

  /// Execute addressed commands
  ///
  /// \param  addr  Address
  /// \param  chunk Raw data
  /// \return true  Command to own address or 0
  /// \return false Command to other address
  bool executeOperationsAddressed(Address addr,
                                  std::span<uint8_t const> chunk) {
    bool const broadcast{addr.type == Address::Broadcast};
    if (!broadcast && addr != _addrs.primary && addr != _addrs.consist)
      return false;

    countOwnEqualPackets();

    switch (decode_instruction(cbegin(chunk))) {
      case Instruction::UnknownService:  // TODO
        break;
      case Instruction::DecoderControl:
        if (broadcast && !chunk[0uz]) serviceMode(true);
        else decoderControl(chunk);
        break;
      case Instruction::ConsistControl: consistControl(chunk); break;
      case Instruction::AdvancedOperations:
        advancedOperations(addr, chunk);
        break;
      case Instruction::SpeedDirection: speedAndDirection(addr, chunk); break;
      case Instruction::FunctionGroup: functionGroup(addr, chunk); break;
      case Instruction::FeatureExpansion: featureExpansion(addr, chunk); break;
      case Instruction::CvLong: cvLong(chunk, broadcast); break;
      case Instruction::CvShort: cvShort(chunk); break;
      default: break;
    }

    return true;
  }

  /// Execute automatic logon
  ///
  /// \param  addr  Address
  /// \param  chunk Raw data
  /// \return true  Command to own address or 0
  /// \return false Command to other address
  bool executeAutomaticLogon(Address addr, std::span<uint8_t const> chunk) {
    if (addr != 254u) return false;

    switch (chunk[0uz] & 0xF0u) {
      // SELECT
      case 0b1101'0000u:
        BiDi::logonSelect(chunk.subspan<2uz, sizeof(uint32_t)>());
        break;

      // LOGON_ASSIGN
      case 0b1110'0000u:
        // Multi-function decoders (long address)
        if (auto const a13_8{chunk[6uz] & 0x3Fu}; a13_8 < 0x28u)
          BiDi::logonAssign(chunk.subspan<2uz, sizeof(uint32_t)>(),
                            decode_address(&chunk[6uz]));
        // Accessory decoder
        else if (a13_8 < 0x38u) break;
        // Multi-function decoders (short address)
        else if (a13_8 < 0x39u)
          BiDi::logonAssign(chunk.subspan<2u, sizeof(uint32_t)>(),
                            decode_address(&chunk[7u]));
        // Reserved
        else if (a13_8 < 0x3Fu) break;
        // FW update
        else if (a13_8 < 0x40u) break;
        break;

      // LOGON_ENABLE
      case 0b1111'0000u: {
        auto const g{static_cast<uint8_t>(chunk[0uz] & 0b11u)};
        auto const cid{data2uint16(&chunk[1uz])};
        auto const session_id{chunk[3uz]};
        BiDi::logonEnable(g, cid, session_id);
        break;
      }
    }

    return true;
  }

  /// Execute decoder control
  void decoderControl(std::span<uint8_t const>) const {
    // TODO
  }

  /// Execute consist control
  ///
  /// \param  chunk Raw data
  void consistControl(std::span<uint8_t const> chunk) {
    write(19u - 1u, static_cast<uint8_t>(chunk[0uz] << 7u | chunk[1uz]));
  }

  /// Execute advanced operations
  ///
  /// \param  addr  Address
  /// \param  chunk Raw data
  void advancedOperations(uint32_t addr, std::span<uint8_t const> chunk) {
    switch (chunk[0uz]) {
      // Speed, direction and function (currently only mentioned in RCN-212)
      case 0b0011'1100u:  // TODO
        break;

      // 126 speed steps (plus 0)
      case 0b0011'1111u: {
        auto const dir{chunk[1uz] & ztl::make_mask(7u) ? 1 : -1};
        // Halt
        if (!(chunk[1uz] & 0b0111'1111u)) directionNotch(addr, dir, 0);
        // Emergency stop
        else if (!(chunk[1uz] & 0b0111'1110u)) impl().emergencyStop(addr);
        else {
          auto const notch{scale_notch<126>((chunk[1uz] & 0b0111'1111) - 1)};
          directionNotch(addr, dir, notch);
        }
        break;
      }

      // MAN
      case 0b0011'1110u:
        if constexpr (EastWestMan<T>) {
          _man = chunk[1uz] & ztl::make_mask(7u);
          if (auto const dir{chunk[1uz] & ztl::make_mask(6u)   ? 1   // East
                             : chunk[1uz] & ztl::make_mask(5u) ? -1  // West
                                                               : 0})
            impl().eastWestMan(addr, dir);
          else impl().eastWestMan(addr, {});
        }
        break;

      // Analog function group
      case 0b0011'1101u:
        // TODO
        break;
    }
  }

  /// Execute speed and direction
  ///
  /// \param  addr  Address
  /// \param  chunk Raw data
  void speedAndDirection(uint32_t addr, std::span<uint8_t const> chunk) {
    auto const dir{chunk[0uz] & ztl::make_mask(5u) ? 1 : -1};
    int32_t notch{};

    // Halt
    if (!(chunk[0uz] & 0b0000'1111u)) notch = 0;
    // Emergency stop
    else if (!(chunk[0uz] & 0b0000'1110u)) return impl().emergencyStop(addr);
    else notch = (chunk[0uz] & 0b0000'1111) - 1;

    // 14 speed steps and F0
    if (_f0_exception) {
      notch = scale_notch<14>(notch);
      auto const mask{ztl::make_mask(0u)};
      auto const state{chunk[0uz] & ztl::make_mask(4u) ? ztl::make_mask(0u)
                                                       : 0u};
      impl().function(addr, mask, state);
    }
    // 28 speed steps
    else {
      notch <<= 1u;
      if (notch && !(chunk[0uz] & ztl::make_mask(4u))) --notch;
      notch = scale_notch<28>(notch);
    }

    directionNotch(addr, dir, notch);
  }

  /// Execute function group
  ///
  /// This function receives the function output states for F0-F12 in three
  /// different sets. The command is one byte long.
  ///
  /// \param  addr  Address
  /// \param  chunk Raw data
  void functionGroup(uint32_t addr, std::span<uint8_t const> chunk) {
    uint32_t mask{};
    uint32_t state{};

    switch (chunk[0uz] & 0xF0u) {
      case 0b1000'0000u: [[fallthrough]];
      case 0b1001'0000u:
        // x-x-x-F0-F4-F3-F2-F1
        mask = _f0_exception ? ztl::make_mask(4u, 3u, 2u, 1u)
                             : ztl::make_mask(4u, 3u, 2u, 1u, 0u);
        state =
          (chunk[0uz] & 0xFu) << 1u | (chunk[0uz] & ztl::make_mask(4u)) >> 4u;
        break;

      case 0b1011'0000u:
        // x-x-x-x-F8-F7-F6-F5
        mask = ztl::make_mask(8u, 7u, 6u, 5u);
        state = (chunk[0uz] & 0x0Fu) << 5u;
        break;

      case 0b1010'0000u:
        // x-x-x-x-F12-F11-F10-F9
        mask = ztl::make_mask(12u, 11u, 10u, 9u);
        state = (chunk[0uz] & 0x0Fu) << 9u;
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
  /// \param  chunk Raw data
  void featureExpansion(uint32_t addr, std::span<uint8_t const> chunk) {
    switch (chunk[0uz]) {
      // Binary state control instruction long form (3 bytes)
      case 0b1100'0000u:
        binaryState((static_cast<uint32_t>(chunk[2uz]) << 7u) |
                      (chunk[1uz] & 0b0111'1111u),
                    chunk[1uz] & ztl::make_mask(7u));
        break;

      // Binary state control instruction short form (2 bytes)
      case 0b1101'1101u:
        binaryState((chunk[1uz] & 0b0111'1111u),
                    chunk[1uz] & ztl::make_mask(7u));
        break;

      // Time (4 bytes)
      case 0b1100'0001u: time(chunk); break;

      // System time (3 bytes)
      case 0b1100'0010u:
        // TODO
        break;

      // Command station properties (4 bytes)
      case 0b1100'0011u:
        // TODO
        break;

      // F20-F19-F18-F17-F16-F15-F14-F13
      case 0b1101'1110u:
        impl().function(addr,
                        ztl::make_mask(20u, 19u, 18u, 17u, 16u, 15u, 14u, 13u),
                        static_cast<uint32_t>(chunk[1uz] << 13u));
        break;

      // F28-F27-F26-F25-F24-F23-F22-F21
      case 0b1101'1111u:
        impl().function(addr,
                        ztl::make_mask(28u, 27u, 26u, 25u, 24u, 23u, 22u, 21u),
                        static_cast<uint32_t>(chunk[1uz] << 21u));
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

  /// Execute configuration variable access - long form
  ///
  /// \param  chunk     Raw data
  /// \param  broadcast Indicate whether packet is broadcast
  void cvLong(std::span<uint8_t const> chunk, bool broadcast = false) {
    switch (uint32_t const cv_addr{(chunk[0uz] & 0b11u) << 8u | chunk[1uz]};
            static_cast<uint32_t>(chunk[0uz]) >> 2u & 0b11u) {
      // Reserved
      case 0b00u: break;

      // Verify byte
      case 0b01u:
        if constexpr (MduEntry<T>)
          if (broadcast) impl().mduEntry(cv_addr, chunk[2uz]);
        verify(cv_addr, chunk[2uz]);
        break;

      // Write byte
      case 0b11u:
        if (_own_equal_packets_count < 2uz) return;
        else if (_own_equal_packets_count == 2uz) write(cv_addr, chunk[2uz]);
        else verify(cv_addr, chunk[2uz]);
        break;

      // Bit manipulation
      case 0b10u: {
        auto const pos{chunk[2uz] & 0b111u};
        auto const bit{static_cast<bool>(chunk[2uz] & ztl::make_mask(3u))};
        if (!(chunk[2uz] & ztl::make_mask(4u))) verify(cv_addr, bit, pos);
        else if (_own_equal_packets_count == 2uz) write(cv_addr, bit, pos);
        break;
      }
    }
  }

  /// Execute configuration variable access - short form
  ///
  /// \param  chunk Raw data
  void cvShort(std::span<uint8_t const> chunk) {
    switch (chunk[0uz] & 0x0Fu) {
      // Not available for use
      case 0b0000u: break;

      // Acceleration adjustment (CV23)
      case 0b0010u:
        write(23u - 1u, chunk[1uz]);  // Fine if written at once
        break;

      // Deceleration adjustment (CV24)
      case 0b0011u:
        write(24u - 1u, chunk[1uz]);  // Fine if written at once
        break;

      // Extended address 0 and 1 (CV17 and CV18)
      case 0b0100u:
        if (_own_equal_packets_count != 2uz) return;
        write(17u - 1u, static_cast<uint8_t>(0b1100'0000u | chunk[1uz]));
        write(18u - 1u, chunk[2uz]);
        write(29u - 1u, true, 5u);
        break;

      // Index high and index low (CV31 and CV32)
      case 0b0101u:
        if (_own_equal_packets_count != 2uz) return;
        write(31u - 1u, chunk[1uz]);
        write(32u - 1u, chunk[2uz]);
        break;

      case 0b1001u: break;
    }
  }

  /// Byte and bit verify
  ///
  /// \param  cv_addr CV address
  /// \param  ts...   CV value or bit and bit position
  void verify(uint32_t cv_addr, auto... ts) {
    if (_cvs_locked) return;
    verifyImpl(cv_addr, ts...);
  }

  /// Byte verify
  ///
  /// \param  cv_addr CV address
  /// \param  byte    CV value
  void verifyImpl(uint32_t cv_addr, uint8_t byte) {
    auto cb{[this, byte](uint8_t red_byte) {
      if (_mode == Mode::Operations) BiDi::pom(red_byte);
      else if (byte == red_byte) impl().serviceAck();
    }};
    if constexpr (AsyncReadable<T>) impl().readCv(cv_addr, byte, cb);
    else cb(impl().readCv(cv_addr, byte));
  }

  /// Bit verify
  ///
  /// \param  cv_addr CV address
  /// \param  bit     CV bit
  /// \param  pos     CV bit position
  void verifyImpl(uint32_t cv_addr, bool bit, uint32_t pos) {
    auto cb{[this, bit](bool red_bit) {
      if (_mode == Mode::Operations) BiDi::pom(red_bit);
      else if (bit == red_bit) impl().serviceAck();
    }};
    if constexpr (AsyncReadable<T>) impl().readCv(cv_addr, bit, pos, cb);
    else cb(impl().readCv(cv_addr, bit, pos));
  }

  /// Byte and bit write, if necessary update config
  ///
  /// \param  cv_addr CV address
  /// \param  ts...   CV value or bit and bit position
  void write(uint32_t cv_addr, auto... ts) {
    if (_cvs_locked && cv_addr != 15u - 1u) return;
    writeImpl(cv_addr, ts...);
    updateConfig(cv_addr);
  }

  /// Byte write
  ///
  /// \param  cv_addr CV address
  /// \param  byte    CV value
  void writeImpl(uint32_t cv_addr, uint8_t byte) {
    auto cb{[this, byte](uint8_t red_byte) {
      if (_mode == Mode::Operations) BiDi::pom(red_byte);
      else if (byte == red_byte) impl().serviceAck();
    }};
    if constexpr (AsyncWritable<T>) impl().writeCv(cv_addr, byte, cb);
    else cb(impl().writeCv(cv_addr, byte));
  }

  /// Bit write
  ///
  /// \param  cv_addr CV address
  /// \param  bit     CV bit
  /// \param  pos     CV bit position
  void writeImpl(uint32_t cv_addr, bool bit, uint32_t pos) {
    auto cb{[this, bit](bool red_bit) {
      if (_mode == Mode::Operations) BiDi::pom(red_bit);
      else if (red_bit == bit) impl().serviceAck();
    }};
    if constexpr (AsyncWritable<T>) impl().writeCv(cv_addr, bit, pos, cb);
    else cb(impl().writeCv(cv_addr, bit, pos));
  }

  /// Register mode
  ///
  /// \param  chunk Raw data
  void registerMode(std::span<uint8_t const> chunk) {
    switch (auto const w{chunk[0uz] & 0b1000u}, reg{chunk[0uz] & 0b111u}; reg) {
      case 0u: [[fallthrough]];
      case 1u: [[fallthrough]];
      case 2u: [[fallthrough]];
      case 3u: {
        auto const i{_index_reg * 4u - (4u - reg)};
        w ? write(i, chunk[1uz]) : verify(i, chunk[1uz]);
        break;
      }
      // CV29
      case 4u:
        w ? write(29u - 1u, chunk[1uz]) : verify(29u - 1u, chunk[1uz]);
        break;
      // Index register
      case 5u:
        if (w) _index_reg = chunk[1uz];
        else if (_index_reg == chunk[1uz]) impl().serviceAck();
        break;
      // CV7
      case 6u: [[fallthrough]];
      // CV8
      case 7u: w ? write(reg, chunk[1uz]) : verify(reg, chunk[1uz]); break;
    }
  }

  /// Time
  void time(std::span<uint8_t const>) const {
    // TODO
  }

  /// Execute binary state
  ///
  /// \param  xf    Number of binary state
  /// \param  state Binary state
  void binaryState(uint32_t xf, bool state) {
    switch (xf) {
      case 2u:
        if (!state) BiDi::tipOffSearch();
        break;
      case 4u: break;
      case 5u: break;
    }
  }

  /// Set direction and notch
  ///
  /// \param  addr  Address
  /// \param  dir   Direction
  /// \param  notch Notch
  void directionNotch(uint32_t addr, int32_t dir, int32_t notch) {
    auto const reverse{addr == _addrs.primary
                         ? impl().readCv(29u - 1u) & ztl::make_mask(0u)
                         : impl().readCv(19u - 1u) & ztl::make_mask(7u)};
    impl().direction(addr, reverse ? dir * -1 : dir);
    impl().notch(addr, notch);
  }

  /// Update quality of service (roughly every second)
  ///
  /// \return Quality of service
  void qos() {
    if (_preamble_count < 100u) return;
    BiDi::qos(
      static_cast<uint8_t>(100u - (_packet_count * 100u) / _preamble_count));
    _packet_count = _preamble_count = 0u;
  }

  /// Count own equal packets
  void countOwnEqualPackets() {
    if (_deque.front() == _last_own_packet) ++_own_equal_packets_count;
    else {
      _own_equal_packets_count = 1uz;
      _last_own_packet = _deque.front();
    }
  }

  /// Flush the current packet
  void flush() { *end(_deque) = {}; }

  /// Reset
  void reset() {
    flush();
    _bit_count = _byte_count = _checksum = 0u;
    _state = State::Preamble;
  }

  /// Enter or exit service mode
  ///
  /// \param  enter Enter service mode
  void serviceMode(bool enter) {
    // Disable other peripherals which might interfere
    if (enter) {
      impl().serviceModeHook(true);
      _mode = Mode::Service;
    } else {
      impl().serviceModeHook(false);
      _mode = Mode::Operations;
    }
  }

  ztl::inplace_deque<Packet, DCC_RX_DEQUE_SIZE> _deque{};
  Packet _last_own_packet{};  ///< Copy of last packet for own address
  size_t _bit_count{};
  size_t _byte_count{};
  size_t _own_equal_packets_count{1uz};
  size_t _packet_count{};
  size_t _preamble_count{};
  uint8_t _checksum{};     ///< On-the-fly calculated checksum
  uint8_t _index_reg{1u};  ///< Paged mode index register
  enum class State : uint8_t { Preamble, Data, Endbit } _state{};
  enum class Mode : uint8_t { Operations, Service } _mode{};

  // Not bitfields as those are most likely mutated in interrupt context
  bool _is_halfbit{};
  bool _packet_end{};

  bool _enabled : 1 {};
  bool _cvs_locked : 1 {};
  bool _f0_exception : 1 {};
  bool _man : 1 {};
  bool _zimo : 1 {};
};

}  // namespace dcc::rx