#include "packets.hpp"
#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>
#include <bitset>
#include <format>
#include <numeric>
#include "state.hpp"
#include "utility.hpp"

#define PRE_COL ImPlot::GetColormapColor(8)      // white
#define START_COL ImPlot::GetColormapColor(2)    // green
#define ADDR_COL ImPlot::GetColormapColor(5)     // yellow
#define DATA_COL ImPlot::GetColormapColor(4)     // orange
#define END_COL ImPlot::GetColormapColor(9)      // red
#define CHECKSUM_COL ImPlot::GetColormapColor(3) // violet

namespace {

namespace eval {

// clang-format off
void eval(State::Packet& packet);
  void preamble(State::Packet& packet);
  void address(State::Packet& packet);
  void instruction(State::Packet& packet);
    void unknown(State::Packet& packet, std::span<uint8_t const> bytes);
    void decoder_control(State::Packet& packet, std::span<uint8_t const> bytes);
      void decoder_control_digital_decoder_reset(State::Packet& packet, std::span<uint8_t const> bytes);
      void decoder_control_hard_reset(State::Packet& packet, std::span<uint8_t const> bytes);
      void decoder_control_factory_test(State::Packet& packet, std::span<uint8_t const> bytes);
      void decoder_control_set_advanced_addressing(State::Packet& packet, std::span<uint8_t const> bytes);
      void decoder_control_decoder_acknowledgement_request(State::Packet& packet, std::span<uint8_t const> bytes);
    void consist_control(State::Packet& packet, std::span<uint8_t const> bytes);
      void consist_control_set_consist_address(State::Packet& packet, std::span<uint8_t const> bytes);
    void advanced_operations(State::Packet& packet, std::span<uint8_t const> bytes);
      void advanced_operations_speed_direction_and_functions(State::Packet& packet, std::span<uint8_t const> bytes);
      void advanced_operations_analog_function_group(State::Packet& packet, std::span<uint8_t const> bytes);
      void advanced_operations_special_operating_modes(State::Packet& packet, std::span<uint8_t const> bytes);
      void advanced_operations_128_speed_step_control(State::Packet& packet, std::span<uint8_t const> bytes);
    void speed_and_direction(State::Packet& packet, std::span<uint8_t const> bytes);
    void function_group(State::Packet& packet, std::span<uint8_t const> bytes);
      void function_group_f0_f4(State::Packet& packet, std::span<uint8_t const> bytes);
      void function_group_f9_f12(State::Packet& packet, std::span<uint8_t const> bytes);
      void function_group_f5_f8(State::Packet& packet, std::span<uint8_t const> bytes);
    void feature_expansion(State::Packet& packet, std::span<uint8_t const> bytes);
      void feature_expansion_binary_state_control_long_form(State::Packet& packet, std::span<uint8_t const> bytes);
      void feature_expansion_time_and_date(State::Packet& packet, std::span<uint8_t const> bytes);
      void feature_expansion_system_time(State::Packet& packet, std::span<uint8_t const> bytes);
      void feature_expansion_command_station_feature_identification(State::Packet& packet, std::span<uint8_t const> bytes);
      void feature_expansion_f29_f36(State::Packet& packet, std::span<uint8_t const> bytes);
      void feature_expansion_f37_f44(State::Packet& packet, std::span<uint8_t const> bytes);
      void feature_expansion_f45_f52(State::Packet& packet, std::span<uint8_t const> bytes);
      void feature_expansion_f53_f60(State::Packet& packet, std::span<uint8_t const> bytes);
      void feature_expansion_f61_f68(State::Packet& packet, std::span<uint8_t const> bytes);
      void feature_expansion_binary_state_control_short_form(State::Packet& packet, std::span<uint8_t const> bytes);
      void feature_expansion_f13_f20(State::Packet& packet, std::span<uint8_t const> bytes);
      void feature_expansion_f21_f28(State::Packet& packet, std::span<uint8_t const> bytes);
    void cv_access(State::Packet& packet, std::span<uint8_t const> bytes);
      void cv_access_long_form(State::Packet& packet, std::span<uint8_t const> bytes);
      void cv_access_short_form(State::Packet& packet, std::span<uint8_t const> bytes);
      void cv_access_xpom(State::Packet& packet, std::span<uint8_t const> bytes);
    void digital_decoder_idle(State::Packet& packet, std::span<uint8_t const> bytes);
    void basic_accessory_decoder_control(State::Packet& packet, std::span<uint8_t const> bytes);
    void extended_accessory_decoder_control(State::Packet& packet, std::span<uint8_t const> bytes);
    void nop_for_basic_and_extended_accessory(State::Packet& packet, std::span<uint8_t const> bytes);
  void checksum(State::Packet& packet);
  void highlights(State::Packet& packet);
  void tags(State::Packet& packet);
// clang-format on

} // namespace eval

namespace tab {

// clang-format off
void tab(State::Packet& packet, size_t i);
  void description(State::Packet& packet);
  void data(State::Packet& packet);
  void plot(State::Packet& packet);
// clang-format on

} // namespace tab

namespace eval {

// Evaluate packet
void eval(State::Packet& packet) {
  if (!empty(packet.desc_strs) && state.cfg == packet.cfg) return;

  // Make a copy of global config to know when to rebuild
  packet.cfg = state.cfg;

  // Need additional bit1 duration to make it end properly
  auto timings{dcc::tx::bytes2timings(packet.bytes, packet.cfg)};
  timings.push_back(packet.cfg.bit1_duration);

  // Clear/resize
  packet.desc_strs.clear();
  packet.pattern_str.clear();
  packet.plots.t.resize(size(timings) * 2uz);
  packet.plots.p.resize(size(timings) * 2uz);
  packet.plots.n.resize(size(timings) * 2uz);
  packet.plots.highlights.resize(size(timings) / 2uz);
  packet.plots.tags.clear();

  // Generate x-axis values for plots
  dcc::tx::Timings::value_type t{};
  for (auto i{0uz}; i < size(timings); ++i) {
    auto const j{i * 2uz};
    packet.plots.t[j] = packet.plots.t[j + 1uz] = static_cast<double>(t);
    auto const bit{static_cast<bool>(i % 2uz)};
    packet.plots.p[j] = bit;
    packet.plots.p[j + 1uz] = !bit;
    packet.plots.n[j] = !bit;
    packet.plots.n[j + 1uz] = bit;
    t += timings[static_cast<dcc::tx::Timings::size_type>(i)];
  }

  preamble(packet);
  address(packet);
  instruction(packet);
  checksum(packet);
  highlights(packet);
  tags(packet);
}

// Annotate preamble
void preamble(State::Packet& packet) {
  packet.pattern_str += std::string(packet.cfg.num_preamble, 'S');
}

// Annotate address
void address(State::Packet& packet) {
  switch (packet.addr.type) {
    case dcc::Address::UnknownService:
      packet.desc_strs.push_back("Service");
      break;
    case dcc::Address::Broadcast:
      packet.desc_strs.push_back(
        std::format("Broadcast={}", packet.addr.value));
      packet.pattern_str += " 0 00000000";
      break;
    case dcc::Address::BasicLoco:
      packet.desc_strs.push_back(
        std::format("Basic Loco={}", packet.addr.value));
      packet.pattern_str += " 0 0AAAAAAA";
      break;
    case dcc::Address::BasicAccessory:
      packet.desc_strs.push_back(
        std::format("Basic Accessory={}", packet.addr.value));
      packet.pattern_str += " 0 10AAAAAA 0 1ÂÂÂDAAR";
      break;
    case dcc::Address::ExtendedAccessory:
      packet.desc_strs.push_back(
        std::format("Extended Accessory={}", packet.addr.value));
      packet.pattern_str += " 0 10AAAAAA 0 0ÂÂÂ0AA1";
      break;
    case dcc::Address::ExtendedLoco:
      packet.desc_strs.push_back(
        std::format("Extended Loco={}", packet.addr.value));
      packet.pattern_str += " 0 11AAAAAA 0 AAAAAAAA";
      break;
    case dcc::Address::Reserved:
      packet.desc_strs.push_back(std::format("Reserved={}", packet.addr.value));
      packet.pattern_str += " 0 11x1xxx";
      break;
    case dcc::Address::DataTransfer:
      packet.desc_strs.push_back(
        std::format("Data Transfer={}", packet.addr.value));
      packet.pattern_str += " 0 11111101";
      break;
    case dcc::Address::AutomaticLogon:
      packet.desc_strs.push_back(
        std::format("Automatic Logon={}", packet.addr.value));
      packet.pattern_str += " 0 11111110";
      break;
    case dcc::Address::Idle:
      packet.desc_strs.push_back(std::format("Idle={}", packet.addr.value));
      packet.pattern_str += " 0 11111111";
      break;
  }
}

// Annotate instruction
void instruction(State::Packet& packet) {
  auto first{cbegin(packet.bytes) +
             (packet.bytes[0uz] >= 128u && packet.bytes[0uz] <= 252u ? 2 : 1)};
  std::span<uint8_t const> bytes{first, cend(packet.bytes)};
  if (empty(bytes)) return;
  else if (packet.addr.type == dcc::Address::UnknownService) {
    packet.desc_strs.push_back("CV Access");
    cv_access_long_form(packet, {cbegin(packet.bytes), cend(packet.bytes)});
  } else if (packet.addr.type == dcc::Address::Broadcast ||
             packet.addr.type == dcc::Address::BasicLoco ||
             packet.addr.type == dcc::Address::ExtendedLoco)
    switch (dcc::decode_instruction(bytes)) {
      case dcc::Instruction::UnknownService: return unknown(packet, bytes);
      case dcc::Instruction::DecoderControl:
        return decoder_control(packet, bytes);
      case dcc::Instruction::ConsistControl:
        return consist_control(packet, bytes);
      case dcc::Instruction::AdvancedOperations:
        return advanced_operations(packet, bytes);
      case dcc::Instruction::SpeedDirection:
        return speed_and_direction(packet, bytes);
      case dcc::Instruction::FunctionGroup:
        return function_group(packet, bytes);
      case dcc::Instruction::FeatureExpansion:
        return feature_expansion(packet, bytes);
      case dcc::Instruction::CvAccess: return cv_access(packet, bytes);
      case dcc::Instruction::Logon: break;
    }
  else if (packet.addr.type == dcc::Address::BasicAccessory)
    packet.bytes[1uz] & ztl::mask<7u>
      ? basic_accessory_decoder_control(packet, bytes)
      : nop_for_basic_and_extended_accessory(packet, bytes);
  else if (packet.addr.type == dcc::Address::ExtendedAccessory)
    packet.bytes[1uz] & ztl::mask<3u>
      ? nop_for_basic_and_extended_accessory(packet, bytes)
      : extended_accessory_decoder_control(packet, bytes);
  else if (packet.addr.type == dcc::Address::Idle)
    digital_decoder_idle(packet, bytes);
}

// Annotate unknown
void unknown(State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.push_back("Unknown");
  for (auto i{0uz}; i < size(bytes) - 1uz; ++i)
    packet.pattern_str += " 0 ????????";
}

// Decoder control
void decoder_control(State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.push_back("Decoder Control");
  switch (bytes[0uz]) {
    case 0b0000'0000u:
      return decoder_control_digital_decoder_reset(packet, bytes);
    case 0b0000'0001u: return decoder_control_hard_reset(packet, bytes);
    case 0b0000'0010u: [[fallthrough]];
    case 0b0000'0011u: return decoder_control_factory_test(packet, bytes);
    case 0b0000'1010u: [[fallthrough]];
    case 0b0000'1011u:
      return decoder_control_set_advanced_addressing(packet, bytes);
    case 0b0000'1111u:
      return decoder_control_decoder_acknowledgement_request(packet, bytes);
  }
}

// Decoder control - digital decoder reset
void decoder_control_digital_decoder_reset(State::Packet& packet,
                                           std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Digital Decoder Reset";
  packet.pattern_str += std::format(" 0 {:08b}", bytes[0uz]);
}

// Decoder control - hard reset
void decoder_control_hard_reset(State::Packet& packet,
                                std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Hard Reset";
  packet.pattern_str += std::format(" 0 {:08b}", bytes[0uz]);
}

// Decoder control - factory test
void decoder_control_factory_test(State::Packet& packet,
                                  std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Factory Test";
  packet.pattern_str += " 0 0000000x";
  for (auto i{1uz}; i < size(bytes) - 1uz; ++i)
    packet.pattern_str += " 0 xxxxxxxx";
}

// Decoder control - set advanced addressing
void decoder_control_set_advanced_addressing(State::Packet& packet,
                                             std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Set Advanced Addressing";
  packet.desc_strs.back() +=
    std::format("\n- CV29:5={}", bytes[0uz] & ztl::mask<0u> ? 1 : 0);
  packet.pattern_str += " 0 0000101D";
}

// Decoder control - decoder acknowledgement request
void decoder_control_decoder_acknowledgement_request(
  State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Decoder Acknowledgement Request";
  packet.pattern_str += std::format(" 0 {:08b}", bytes[0uz]);
}

// Consist control
void consist_control(State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.push_back("Consist Control");
  switch (bytes[0uz]) {
    case 0b0001'0010: [[fallthrough]];
    case 0b0001'0011: return consist_control_set_consist_address(packet, bytes);
  }
}

// Consist control - set consist address
void consist_control_set_consist_address(State::Packet& packet,
                                         std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Set Consist Address";
  packet.desc_strs.back() +=
    std::format("\n- Reversed={}", bytes[0uz] & ztl::mask<0u> ? 1 : 0);
  packet.desc_strs.back() += std::format("\n- Address={}", bytes[1uz]);
  packet.pattern_str += " 0 0001001R 0 0AAAAAAA";
}

// Advanced operations
void advanced_operations(State::Packet& packet,
                         std::span<uint8_t const> bytes) {
  packet.desc_strs.push_back("Advanced Operations");
  switch (bytes[0uz]) {
    case 0b0011'1100u:
      return advanced_operations_speed_direction_and_functions(packet, bytes);
    case 0b0011'1101u:
      return advanced_operations_analog_function_group(packet, bytes);
    case 0b0011'1110u:
      return advanced_operations_special_operating_modes(packet, bytes);
    case 0b0011'1111u:
      return advanced_operations_128_speed_step_control(packet, bytes);
  }
}

// Advanced operations - speed, direction and functions
void advanced_operations_speed_direction_and_functions(
  State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Speed, Direction and Functions";
  packet.desc_strs.back() +=
    std::format("\n- Direction={}", bytes[1uz] & ztl::mask<7u> ? 1 : 0);
  packet.desc_strs.back() += std::format(
    "\n- Speed={}",
    speed_labels[static_cast<size_t>(dcc::decode_rggggggg(bytes[1uz]) + 1)]);
  packet.desc_strs.back() += std::format("\n- F7-F0={:08b}", bytes[2uz]);
  if (size(bytes) >= 4uz + 1uz)
    packet.desc_strs.back() += std::format("\n- F15-F8={:08b}", bytes[3uz]);
  if (size(bytes) >= 5uz + 1uz)
    packet.desc_strs.back() += std::format("\n- F23-F16={:08b}", bytes[4uz]);
  if (size(bytes) >= 6uz + 1uz)
    packet.desc_strs.back() += std::format("\n- F31-F24={:08b}", bytes[5uz]);
  packet.pattern_str += " 0 00111100 0 RGGGGGGG 0 DDDDDDDD";
  for (auto i{3uz}; i < size(bytes) - 1uz; ++i)
    packet.pattern_str += " 0 DDDDDDDD";
}

// Advanced operations - analog function group
void advanced_operations_analog_function_group(State::Packet& packet,
                                               std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Analog Function Group";
  packet.desc_strs.back() +=
    std::format("\n- Channel={}", analog_labels[bytes[1uz]]);
  packet.desc_strs.back() += std::format("\n- Value={}", bytes[2uz]);
  packet.pattern_str += " 0 00111101 0 SSSSSSSS 0 DDDDDDDD";
}

// Advanced operations - special operating modes
void advanced_operations_special_operating_modes(
  State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Special Operating Modes";
  auto const cc{(bytes[1uz] >> 2u) & 0b11u};
  packet.desc_strs.back() +=
    std::format("\n- Consist={}",
                consist_labels[((cc & 1) << 1) | ((cc & 2) >> 1) | (cc & ~3)]);
  packet.desc_strs.back() +=
    std::format("\n- Shunting={}", bytes[1uz] & ztl::mask<4u> ? 1 : 0);
  packet.desc_strs.back() +=
    std::format("\n- West={}", bytes[1uz] & ztl::mask<5u> ? 1 : 0);
  packet.desc_strs.back() +=
    std::format("\n- East={}", bytes[1uz] & ztl::mask<6u> ? 1 : 0);
  packet.desc_strs.back() +=
    std::format("\n- MAN={}", bytes[1uz] & ztl::mask<7u> ? 1 : 0);
  packet.pattern_str += " 0 00111110 0 DDDDDD00";
}

// Advanced operations - 128 speed step control
void advanced_operations_128_speed_step_control(
  State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - 128 Speed Step Control";
  packet.desc_strs.back() +=
    std::format("\n- Direction={}", bytes[1uz] & ztl::mask<7u> ? 1 : 0);
  packet.desc_strs.back() += std::format(
    "\n- Speed={}",
    speed_labels[static_cast<size_t>(dcc::decode_rggggggg(bytes[1uz]) + 1)]);
  packet.pattern_str += " 0 00111111 0 RGGGGGGG";
}

// Speed and Direction
void speed_and_direction(State::Packet& packet,
                         std::span<uint8_t const> bytes) {
  packet.desc_strs.push_back("Speed & Direction");
  packet.desc_strs.back() +=
    std::format("\n- Direction={}", bytes[0uz] & ztl::mask<5u> ? 1 : 0);
  packet.desc_strs.back() +=
    std::format("\n- Speed={}",
                speed_labels[static_cast<size_t>(
                  dcc::decode_rggggg(bytes[0uz], true) + 1)]);
  packet.desc_strs.back() +=
    std::format(" or Speed={} and F0={}",
                speed_labels[static_cast<size_t>(
                  dcc::decode_rggggg(bytes[0uz], false) + 1)],
                bytes[0uz] & ztl::mask<4u> ? 1 : 0);
  packet.pattern_str += " 0 01RGGGGG";
}

// Function group
void function_group(State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.push_back("Function Group");
  switch (bytes[0uz] & 0xF0u) {
    case 0b1000'0000u: [[fallthrough]];
    case 0b1001'0000u: return function_group_f0_f4(packet, bytes);
    case 0b1010'0000u: return function_group_f9_f12(packet, bytes);
    case 0b1011'0000u: return function_group_f5_f8(packet, bytes);
  }
}

// Function group - F0-F4
void function_group_f0_f4(State::Packet& packet,
                          std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - F0-F4";
  packet.desc_strs.back() +=
    std::format("\n- F4-F0={:05b}",
                ((bytes[0uz] & 0x0Fu) << 1u) | ((bytes[0uz] & 0x1Fu) >> 4u));
  packet.pattern_str += " 0 100DDDDD";
}

// Function group - F9-F12
void function_group_f9_f12(State::Packet& packet,
                           std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - F9-F12";
  packet.desc_strs.back() +=
    std::format("\n- F12-F9={:04b}", bytes[0uz] & 0x0Fu);
  packet.pattern_str += " 0 1010DDDD";
}

// Function group - F5-F8
void function_group_f5_f8(State::Packet& packet,
                          std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - F5-F8";
  packet.desc_strs.back() +=
    std::format("\n- F8-F5={:04b}", bytes[0uz] & 0x0Fu);
  packet.pattern_str += " 0 1011DDDD";
}

// Feature expansion
void feature_expansion(State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.push_back("Feature Expansion");
  switch (bytes[0uz]) {
    case 0b1100'0000u:
      return feature_expansion_binary_state_control_long_form(packet, bytes);
    case 0b1100'0001u: return feature_expansion_time_and_date(packet, bytes);
    case 0b1100'0010u: return feature_expansion_system_time(packet, bytes);
    case 0b1100'0011u:
      return feature_expansion_command_station_feature_identification(packet,
                                                                      bytes);
    case 0b1101'1000u: return feature_expansion_f29_f36(packet, bytes);
    case 0b1101'1001u: return feature_expansion_f37_f44(packet, bytes);
    case 0b1101'1010u: return feature_expansion_f45_f52(packet, bytes);
    case 0b1101'1011u: return feature_expansion_f53_f60(packet, bytes);
    case 0b1101'1100u: return feature_expansion_f61_f68(packet, bytes);
    case 0b1101'1101u:
      return feature_expansion_binary_state_control_short_form(packet, bytes);
    case 0b1101'1110u: return feature_expansion_f13_f20(packet, bytes);
    case 0b1101'1111u: return feature_expansion_f21_f28(packet, bytes);
  }
}

// Feature expansion - binary state control long form
void feature_expansion_binary_state_control_long_form(
  State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Binary State Control Long Form";
  packet.desc_strs.back() +=
    std::format("\n- Address={}", (bytes[2uz] << 7u) | (bytes[1uz] & 0x7Fu));
  packet.desc_strs.back() +=
    std::format("\n- State={}", bytes[1uz] & ztl::mask<7u> ? 1 : 0);
  packet.pattern_str += " 0 11000000 0 DLLLLLLL 0 HHHHHHHH";
}

// Feature expansion - time and date
void feature_expansion_time_and_date(State::Packet& packet,
                                     std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Time and Date";
  switch (bytes[1uz] >> 6u) {
    case 0b00u:
      packet.desc_strs.back() += std::format("\n- Minutes={}", bytes[1uz]);
      packet.desc_strs.back() +=
        std::format("\n- Weekday={}", weekday_labels[bytes[2uz] >> 5u]);
      packet.desc_strs.back() +=
        std::format("\n- Hours={}", bytes[2uz] & 0x1Fu);
      packet.desc_strs.back() +=
        std::format("\n- Update={}", bytes[3uz] & ztl::mask<7u> ? 1 : 0);
      packet.desc_strs.back() +=
        std::format("\n- Acceleration={}", bytes[3uz] & 0x3Fu);
      packet.pattern_str += " 0 11000001 0 00MMMMMM 0 WWWHHHHH 0 U0BBBBBB";
      break;
    case 0b01u:
      packet.desc_strs.back() += std::format("\n- Day={}", bytes[1uz] & 0x1Fu);
      packet.desc_strs.back() += std::format("\n- Month={}", bytes[2uz] >> 4u);
      packet.desc_strs.back() +=
        std::format("\n- Year={}", ((bytes[2uz] & 0x0Fu) << 8u) | bytes[3uz]);
      packet.pattern_str += " 0 11000001 0 010TTTTT 0 MMMMYYYY 0 YYYYYYYY";
      break;
    case 0b10u:
#if defined(__STDCPP_FLOAT16_T__)
      std::float16_t f16;
      std::array buf{bytes[3uz], bytes[2uz]};
      memcpy(&f16, data(buf), sizeof(f16));
#else
      auto const f16{
        dcc::float16_to_float32(bytes[2uz] << 8u | bytes[3uz] << 0u)};
#endif
      packet.desc_strs.back() +=
        std::format("\n- Time Scale={}", static_cast<float>(f16));
      packet.pattern_str += " 0 11000001 0 10111111 0 SEEEEEMM 0 MMMMMMMM";
      break;
  }
}

// Feature expansion - system time
void feature_expansion_system_time(State::Packet& packet,
                                   std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - System Time";
  packet.desc_strs.back() +=
    std::format("\n- Milliseconds={}", (bytes[1uz] << 8u) | bytes[2uz]);
  packet.pattern_str += " 0 11000010 0 MMMMMMMM 0 MMMMMMMM";
}

// Feature expansion - command station feature identification
void feature_expansion_command_station_feature_identification(
  State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Command Station Feature Identification";
  switch (auto const iiii{bytes[1uz] & 0x0Fu}) {
    case 0b1111u: {
      dcc::LocoFeatures const feats{
        static_cast<uint16_t>(bytes[2uz] << 8u | bytes[3uz] << 0u)};
      packet.desc_strs.back() += "\n- Loco Features";
      packet.desc_strs.back() += std::format(
        "\n- Basic Addresses 100-127 as Extended={}",
        static_cast<bool>(feats &
                          dcc::LocoFeatures::BasicAddresses100_127AsExtended)
          ? 0
          : 1);
      packet.desc_strs.back() +=
        std::format("\n- Extended Addresses 10000-10239={}",
                    static_cast<bool>(
                      feats & dcc::LocoFeatures::ExtendedAddresses10000_10239)
                      ? 0
                      : 1);
      packet.desc_strs.back() += std::format(
        "\n- 128 Speed Steps={}",
        static_cast<bool>(feats & dcc::LocoFeatures::SpeedSteps128) ? 0 : 1);
      packet.desc_strs.back() +=
        std::format("\n- Speed, Direction and Functions={}",
                    static_cast<bool>(feats & dcc::LocoFeatures::SDF) ? 0 : 1);
      packet.desc_strs.back() += std::format(
        "\n- POM Write={}",
        static_cast<bool>(feats & dcc::LocoFeatures::PomWrite) ? 0 : 1);
      packet.desc_strs.back() += std::format(
        "\n- XPOM Write={}",
        static_cast<bool>(feats & dcc::LocoFeatures::XpomWrite) ? 0 : 1);
      packet.desc_strs.back() += std::format(
        "\n- F13-F28={}",
        static_cast<bool>(feats & dcc::LocoFeatures::F13_F28) ? 0 : 1);
      packet.desc_strs.back() += std::format(
        "\n- F29-F68={}",
        static_cast<bool>(feats & dcc::LocoFeatures::F29_F68) ? 0 : 1);
      packet.desc_strs.back() += std::format(
        "\n- Binary State Short={}",
        static_cast<bool>(feats & dcc::LocoFeatures::BinaryStateShort) ? 0 : 1);
      packet.desc_strs.back() += std::format(
        "\n- Binary State Long={}",
        static_cast<bool>(feats & dcc::LocoFeatures::BinaryStateLong) ? 0 : 1);
      packet.desc_strs.back() += std::format(
        "\n- Analog Function={}",
        static_cast<bool>(feats & dcc::LocoFeatures::AnalogFunction) ? 0 : 1);
      packet.desc_strs.back() += std::format(
        "\n- Special Operating Modes={}",
        static_cast<bool>(feats & dcc::LocoFeatures::SpecialOperatingModes)
          ? 0
          : 1);
      break;
    }
    case 0b1110u: {
      dcc::AccessoryBroadcastFeatures const feats{
        static_cast<uint16_t>(bytes[2uz] << 8u | bytes[3uz] << 0u)};
      packet.desc_strs.back() += "\n- Accessory and Broadcast Features";
      packet.desc_strs.back() += std::format(
        "\n- Addresses Offset by 4={}",
        static_cast<bool>(feats &
                          dcc::AccessoryBroadcastFeatures::AddressesOffsetBy4)
          ? 0
          : 1);
      packet.desc_strs.back() += std::format(
        "\n- Extended={}",
        static_cast<bool>(feats & dcc::AccessoryBroadcastFeatures::Extended)
          ? 0
          : 1);
      packet.desc_strs.back() += std::format(
        "\n- POM Write={}",
        static_cast<bool>(feats & dcc::AccessoryBroadcastFeatures::PomWrite)
          ? 0
          : 1);
      packet.desc_strs.back() += std::format(
        "\n- Time={}",
        static_cast<bool>(feats & dcc::AccessoryBroadcastFeatures::Time) ? 0
                                                                         : 1);
      packet.desc_strs.back() += std::format(
        "\n- Date={}",
        static_cast<bool>(feats & dcc::AccessoryBroadcastFeatures::Date) ? 0
                                                                         : 1);
      packet.desc_strs.back() += std::format(
        "\n- Time Scale={}",
        static_cast<bool>(feats & dcc::AccessoryBroadcastFeatures::TimeScale)
          ? 0
          : 1);
      packet.desc_strs.back() += std::format(
        "\n- System Time={}",
        static_cast<bool>(feats & dcc::AccessoryBroadcastFeatures::SystemTime)
          ? 0
          : 1);
      break;
    }
    case 0b1101u: {
      dcc::BiDiFeatures const feats{
        static_cast<uint16_t>(bytes[2uz] << 8u | bytes[3uz] << 0u)};
      packet.desc_strs.back() += "\n- RailCom Features";
      packet.desc_strs.back() += std::format(
        "\n- RailCom={}",
        static_cast<bool>(feats & dcc::BiDiFeatures::RailCom) ? 0 : 1);
      packet.desc_strs.back() +=
        std::format("\n- DCC-A={}",
                    static_cast<bool>(feats & dcc::BiDiFeatures::DccA) ? 0 : 1);
      packet.desc_strs.back() += std::format(
        "\n- NOP for Accessories={}",
        static_cast<bool>(feats & dcc::BiDiFeatures::NopForAccessories) ? 0
                                                                        : 1);
      packet.desc_strs.back() += std::format(
        "\n- POM Read={}",
        static_cast<bool>(feats & dcc::BiDiFeatures::PomRead) ? 0 : 1);
      packet.desc_strs.back() += std::format(
        "\n- XPOM Read={}",
        static_cast<bool>(feats & dcc::BiDiFeatures::XpomRead) ? 0 : 1);
      packet.desc_strs.back() += std::format(
        "\n- app:dyn Container Levels={}",
        static_cast<bool>(feats & dcc::BiDiFeatures::AppDynContainerLevels)
          ? 0
          : 1);
      packet.desc_strs.back() += std::format(
        "\n- app:dyn Operating Parameters={}",
        static_cast<bool>(feats & dcc::BiDiFeatures::AppDynOperatingParams)
          ? 0
          : 1);
      packet.desc_strs.back() += std::format(
        "\n- app:dyn Track Voltage={}",
        static_cast<bool>(feats & dcc::BiDiFeatures::AppDynTrackVoltage) ? 0
                                                                         : 1);
      packet.desc_strs.back() += std::format(
        "\n- RailCom+={}",
        static_cast<bool>(feats & dcc::BiDiFeatures::RailComPlus) ? 0 : 1);
      break;
    }
  }
  packet.pattern_str += " 0 11000011 0 1111IIII 0 DDDDDDDD 0 DDDDDDDD";
}

// Feature expansion - F29-F36
void feature_expansion_f29_f36(State::Packet& packet,
                               std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - F29-F36";
  packet.desc_strs.back() += std::format("\n- F36-F29={:08b}", bytes[1uz]);
  packet.pattern_str += " 0 11011000 0 DDDDDDDD";
}

// Feature expansion - F37-F44
void feature_expansion_f37_f44(State::Packet& packet,
                               std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - F37-F44";
  packet.desc_strs.back() += std::format("\n- F44-F37={:08b}", bytes[1uz]);
  packet.pattern_str += " 0 11011001 0 DDDDDDDD";
}

// Feature expansion - F45-F52
void feature_expansion_f45_f52(State::Packet& packet,
                               std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - F45-F52";
  packet.desc_strs.back() += std::format("\n- F52-F45={:08b}", bytes[1uz]);
  packet.pattern_str += " 0 11011010 0 DDDDDDDD";
}

// Feature expansion - F53-F60
void feature_expansion_f53_f60(State::Packet& packet,
                               std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - F53-F60";
  packet.desc_strs.back() += std::format("\n- F60-F53={:08b}", bytes[1uz]);
  packet.pattern_str += " 0 11011011 0 DDDDDDDD";
}

// Feature expansion - F61-F68
void feature_expansion_f61_f68(State::Packet& packet,
                               std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - F61-F68";
  packet.desc_strs.back() += std::format("\n- F68-F61={:08b}", bytes[1uz]);
  packet.pattern_str += " 0 11011100 0 DDDDDDDD";
}

// Feature expansion - binary state control short form
void feature_expansion_binary_state_control_short_form(
  State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Binary State Control Short Form";
  packet.desc_strs.back() += std::format("\n- Address={}", bytes[1uz] & 0x7Fu);
  packet.desc_strs.back() +=
    std::format("\n- State={}", bytes[1uz] & ztl::mask<7u> ? 1 : 0);
  packet.pattern_str += " 0 11011101 0 DLLLLLLL";
}

// Feature expansion - F13-F20
void feature_expansion_f13_f20(State::Packet& packet,
                               std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - F13-F20";
  packet.desc_strs.back() += std::format("\n- F20-F13={:08b}", bytes[1uz]);
  packet.pattern_str += " 0 11011110 0 DDDDDDDD";
}

// Feature expansion - F21-F28
void feature_expansion_f21_f28(State::Packet& packet,
                               std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - F21-F28";
  packet.desc_strs.back() += std::format("\n- F28-F21={:08b}", bytes[1uz]);
  packet.pattern_str += " 0 11011111 0 DDDDDDDD";
}

// CV access
void cv_access(State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.push_back("CV Access");
  if (bytes[0uz] & ztl::mask<4u>) cv_access_short_form(packet, bytes);
  else if (size(bytes) <= 3uz + 1uz) cv_access_long_form(packet, bytes);
  else cv_access_xpom(packet, bytes);
}

// CV access - long form
void cv_access_long_form(State::Packet& packet,
                         std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Long Form";
  auto const kk{bytes[0uz] >> 2u & 0b11u};
  auto const cv_addr{(bytes[0uz] & 0b11u) << 8u | bytes[1uz]};
  switch (kk) {
    case 0b01u: [[fallthrough]];
    case 0b11u:
      packet.desc_strs.back() +=
        kk == 0b01u ? "\n- Verify Byte" : "\n- Write Byte";
      packet.desc_strs.back() +=
        std::format("\n- CV{}={}", cv_addr + 1u, bytes[2uz]);
      packet.pattern_str += packet.addr.type == dcc::Address::UnknownService
                              ? " 0 0111KKVV 0 VVVVVVVV 0 DDDDDDDD"
                              : " 0 1110KKVV 0 VVVVVVVV 0 DDDDDDDD";
      break;
    case 0b10u:
      auto const pos{bytes[2uz] & 0b111u};
      auto const bit{bytes[2uz] & ztl::mask<3u>};
      packet.desc_strs.back() +=
        bytes[2uz] & ztl::mask<4u> ? "\n- Write Bit" : "\n- Verify Bit";
      packet.desc_strs.back() +=
        std::format("\n- CV{}:{}={}", cv_addr + 1u, pos, bit ? 0 : 1);
      packet.pattern_str += packet.addr.type == dcc::Address::UnknownService
                              ? " 0 1110KKVV 0 VVVVVVVV 0 111KDBBB"
                              : " 0 0111KKVV 0 VVVVVVVV 0 111KDBBB";
      break;
  }
}

// CV access - short form
void cv_access_short_form(State::Packet& packet,
                          std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Short Form";
  auto const kkkk{bytes[0uz] & 0x0Fu};
  switch (kkkk) {
    case 0b0010u:
      packet.desc_strs.back() += "\n- Write Byte";
      packet.desc_strs.back() += std::format("\n- CV23={}", bytes[1uz]);
      packet.pattern_str += " 0 1111KKKK 0 DDDDDDDD";
      break;
    case 0b0011u:
      packet.desc_strs.back() += "\n- Write Byte";
      packet.desc_strs.back() += std::format("\n- CV24={}", bytes[1uz]);
      packet.pattern_str += " 0 1111KKKK 0 DDDDDDDD";
      break;
    case 0b0100u:
      packet.desc_strs.back() += "\n- Write Bytes";
      packet.desc_strs.back() += std::format("\n- CV17={}", bytes[1uz]);
      packet.desc_strs.back() += std::format("\n- CV18={}", bytes[2uz]);
      packet.pattern_str += " 0 1111KKKK 0 DDDDDDDD 0 DDDDDDDD";
      break;
    case 0b0101u:
      packet.desc_strs.back() += "\n- Write Bytes";
      packet.desc_strs.back() += std::format("\n- CV31={}", bytes[1uz]);
      packet.desc_strs.back() += std::format("\n- CV32={}", bytes[2uz]);
      packet.pattern_str += " 0 1111KKKK 0 DDDDDDDD 0 DDDDDDDD";
      break;
    case 0b0110u:
      packet.desc_strs.back() += "\n- Write Bytes";
      packet.desc_strs.back() += std::format("\n- CV19={}", bytes[1uz]);
      packet.desc_strs.back() += std::format("\n- CV20={}", bytes[2uz]);
      packet.pattern_str += " 0 1111KKKK 0 DDDDDDDD 0 DDDDDDDD";
      break;
  }
}

// CV access - XPOM
void cv_access_xpom(State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - XPOM";
  auto const kk{bytes[0uz] >> 2u & 0b11u};
  auto const ss{bytes[0uz] & 0b11u};
  auto const page_str{
    std::format("Page CV31={} CV32={}", bytes[1uz], bytes[2uz])};
  auto const cv_addr{bytes[1uz] << 16u | bytes[2uz] << 8u | bytes[3uz] << 0u};
  packet.desc_strs.back() += std::format("\n- Sequence Number={:02b}", ss);
  switch (kk) {
    case 0b01u:
      packet.desc_strs.back() += std::format("\n- Verify Bytes @ {}", page_str);
      for (auto i{1uz}; i <= 4uz; ++i)
        packet.desc_strs.back() +=
          std::format("\n- CV{} (CV{})", bytes[3uz] + i, cv_addr + i);
      packet.pattern_str += " 0 1110KKSS 0 VVVVVVVV 0 VVVVVVVV 0 VVVVVVVV";
      break;
    case 0b11u:
      packet.desc_strs.back() += std::format("\n- Write Bytes @ {}", page_str);
      packet.pattern_str += " 0 1110KKSS 0 VVVVVVVV 0 VVVVVVVV 0 VVVVVVVV";
      for (auto i{4uz}; i < size(bytes) - 1uz; ++i) {
        packet.desc_strs.back() += std::format("\n- CV{0}={1} (CV{2}={1})",
                                               bytes[3uz] + 1u + i - 4u,
                                               bytes[i],
                                               cv_addr + 1u + i - 4u);
        packet.pattern_str += " 0 DDDDDDDD";
      }
      break;
    case 0b10u:
      auto const pos{bytes[4uz] & 0b111u};
      auto const bit{bytes[4uz] & ztl::mask<3u>};
      packet.desc_strs.back() += std::format("\n- Write Bit @ {}", page_str);
      packet.desc_strs.back() +=
        std::format("\n- CV{0}:{1}={2} (CV{3}:{1}={2})",
                    bytes[3uz] + 1u,
                    pos,
                    bit ? 1 : 0,
                    cv_addr + 1u);
      packet.pattern_str +=
        " 0 1110KKSS 0 VVVVVVVV 0 VVVVVVVV 0 VVVVVVVV 0 1111DBBB";
      break;
  }
}

// Digital decoder idle
void digital_decoder_idle(State::Packet& packet,
                          std::span<uint8_t const> bytes) {
  packet.desc_strs.push_back("Digital Decoder Idle");
  packet.pattern_str += " 0 00000000";
}

// Basic accessory decoder control
void basic_accessory_decoder_control(State::Packet& packet,
                                     std::span<uint8_t const>) {
  packet.desc_strs.push_back("Basic Accessory Decoder Control");
  packet.desc_strs.back() +=
    std::format("\n- Output={}", packet.bytes[1uz] & ztl::mask<0u> ? 1 : 0);
  packet.desc_strs.back() +=
    std::format("\n- State={}", packet.bytes[1uz] & ztl::mask<3u> ? 1 : 0);
}

// Extended accessory decoder control
void extended_accessory_decoder_control(State::Packet& packet,
                                        std::span<uint8_t const> bytes) {
  packet.desc_strs.push_back("Extended Accessory Decoder Control");
  packet.desc_strs.back() += std::format("\n- Aspect={} or", bytes[0uz]);
  auto const switch_on_time_label{switch_on_time_labels[bytes[0uz] & 0x7Fu]};
  packet.desc_strs.back() += std::format("\n- Output={} for Time={}",
                                         bytes[0uz] & ztl::mask<7u> ? 1 : 0,
                                         switch_on_time_label);
  if (std::string_view{switch_on_time_label}.contains('.'))
    packet.desc_strs.back() += 's';
  packet.pattern_str += " 0 DDDDDDDD";
}

// NOP for basic and extended accessory
void nop_for_basic_and_extended_accessory(State::Packet& packet,
                                          std::span<uint8_t const> bytes) {
  packet.desc_strs.push_back("NOP for Basic and Extended Accessory Decoder");
}

// Annotate checksum
void checksum(State::Packet& packet) {
  packet.desc_strs.push_back("Checksum");
  // Add missing bytes
  while (size(packet.pattern_str) <
         packet.cfg.num_preamble +
           (size(packet.bytes) - 1uz) *
             (CHAR_BIT + sizeof(' ') + sizeof('0') + sizeof(' ')))
    packet.pattern_str += " 0 ????????";
  packet.pattern_str += " 0 PPPPPPPP 1";
}

// Generate highlights for plot
void highlights(State::Packet& packet) {
  auto first{cbegin(packet.bytes) +
             (packet.bytes[0uz] >= 128u && packet.bytes[0uz] <= 252u ? 2 : 1)};
  auto const addr_count{static_cast<size_t>(first - cbegin(packet.bytes))};
  std::span<uint8_t const> bytes{first, cend(packet.bytes)};

  // Build data string
  std::string data_str(packet.cfg.num_preamble, '1');
  for (auto b : packet.bytes) data_str += std::format(" 0 {:08b}", b);
  data_str += " 1";

  // Combine pattern and data string
  auto const hgl_str{packet.pattern_str + "\n" + data_str + "\n"};

  // Highlights index
  size_t i{};

  // Caret index
  size_t c{};

  // Byte index
  size_t k{};

  //
  for (auto l{0uz}; l < packet.cfg.num_preamble; ++l)
    packet.plots.highlights[i++] = {
      PRE_COL,
      hgl_str +
        std::string(size(packet.pattern_str), ' ').replace(c++, 1uz, 1uz, '^')};

  // Address
  if (packet.addr.type != dcc::Address::UnknownService)
    for (; k < addr_count; ++k) {
      // Space
      ++c;

      // Startbit
      packet.plots.highlights[i++] = {
        START_COL,
        hgl_str + std::string(size(packet.pattern_str), ' ')
                    .replace(c++, 1uz, 1uz, '^')};

      // Space
      ++c;

      //
      for (auto l{0uz}; l < CHAR_BIT; ++l)
        packet.plots.highlights[i++] = {
          ADDR_COL,
          hgl_str + std::string(size(packet.pattern_str), ' ')
                      .replace(c++, 1uz, 1uz, '^')};
    }

  // Data
  for (; k < size(packet.bytes) - 1uz; ++k) {
    // Space
    ++c;

    // Startbit
    packet.plots.highlights[i++] = {
      START_COL,
      hgl_str +
        std::string(size(packet.pattern_str), ' ').replace(c++, 1uz, 1uz, '^')};

    // Space
    ++c;

    // Byte
    for (auto l{0uz}; l < CHAR_BIT; ++l)
      packet.plots.highlights[i++] = {
        DATA_COL,
        hgl_str + std::string(size(packet.pattern_str), ' ')
                    .replace(c++, 1uz, 1uz, '^')};
  }

  // Checksum
  for (; k < size(packet.bytes); ++k) {
    // Space
    ++c;

    // Startbit
    packet.plots.highlights[i++] = {
      START_COL,
      hgl_str +
        std::string(size(packet.pattern_str), ' ').replace(c++, 1uz, 1uz, '^')};

    // Space
    ++c;

    // Byte
    for (auto l{0uz}; l < CHAR_BIT; ++l)
      packet.plots.highlights[i++] = {
        CHECKSUM_COL,
        hgl_str + std::string(size(packet.pattern_str), ' ')
                    .replace(c++, 1uz, 1uz, '^')};
  }

  // Space
  ++c;

  // Endbit
  packet.plots.highlights[i++] = {
    END_COL,
    hgl_str +
      std::string(size(packet.pattern_str), ' ').replace(c, 1uz, 1uz, '^')};
}

// Generate tags for plot
void tags(State::Packet& packet) {
  auto first{cbegin(packet.bytes) +
             (packet.bytes[0uz] >= 128u && packet.bytes[0uz] <= 252u ? 2 : 1)};
  std::span<uint8_t const> bytes{first, cend(packet.bytes)};

  auto const num_preamble{packet.cfg.num_preamble};
  auto const bit1_duration{packet.cfg.bit1_duration};
  auto const bit0_duration{packet.cfg.bit0_duration};

  double t{0.0};
  packet.plots.tags.push_back({t, PRE_COL, "Preamble"});
  t += 2.0 * (num_preamble * bit1_duration + bit0_duration);

  if (packet.addr.type != dcc::Address::UnknownService) {
    packet.plots.tags.push_back({t, ADDR_COL, "Address"});
    t += std::accumulate(
      cbegin(packet.bytes),
      first,
      0.0,
      [bit1_duration, bit0_duration](double a, uint8_t b) {
        return a + 2.0 * (bit0_duration + (std::popcount(b) * bit1_duration) +
                          ((CHAR_BIT - std::popcount(b)) * bit0_duration));
      });
  }

  packet.plots.tags.push_back({t, DATA_COL, "Data"});
  t += std::accumulate(
    first,
    cend(packet.bytes) - 1,
    0.0,
    [bit1_duration, bit0_duration](double a, uint8_t b) {
      return a + 2.0 * (bit0_duration + (std::popcount(b) * bit1_duration) +
                        ((CHAR_BIT - std::popcount(b)) * bit0_duration));
    });

  packet.plots.tags.push_back({t, CHECKSUM_COL, "Checksum"});
}

} // namespace eval

////////////////////////////////////////////////////////////////////////////////

namespace tab {

// Tab
void tab(State::Packet& packet, size_t i) {
  if (ImGui::BeginTabItem(("#" + std::to_string(i) + UNIQUE_LABEL()).c_str(),
                          &packet.show,
                          ImGuiTabItemFlags_None)) {
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode(
          (std::string{"Description"} + UNIQUE_LABEL()).c_str())) {
      description(packet);
      ImGui::TreePop();
    }
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode((std::string{"Data"} + UNIQUE_LABEL()).c_str())) {
      data(packet);
      ImGui::TreePop();
    }
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode((std::string{"Plot"} + UNIQUE_LABEL()).c_str())) {
      plot(packet);
      ImGui::TreePop();
    }
    ImGui::EndTabItem();
  }
}

// Description node
void description(State::Packet& packet) {
  for (auto const& desc : packet.desc_strs)
    ImGui::BulletText("%s", desc.c_str());
}

// Data node
void data(State::Packet& packet) {
  ImGui::BinaryTable(UNIQUE_LABEL(),
                     data(packet.bytes),
                     ssize(packet.bytes),
                     ImGuiInputTextFlags_ReadOnly);
}

// Plot node
void plot(State::Packet& packet) {
  if (ImPlot::BeginPlot("Digital Signal")) {
    // Plot P and N
    ImPlot::SetupAxis(ImAxis_X1, "Time [us]");
    ImPlot::SetupAxis(ImAxis_Y1, "P", ImPlotAxisFlags_NoTickLabels);
    ImPlot::SetupAxis(
      ImAxis_Y2, "N", ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_Opposite);
    ImPlot::SetupAxisLimits(
      ImAxis_X1, -0.01 * packet.plots.t.back(), 1.01 * packet.plots.t.back());
    ImPlot::SetupAxisLimits(ImAxis_Y1, -4.0 + 1.0, 2.0);
    ImPlot::SetupAxisLimits(ImAxis_Y2, -2.0 + 1.0, 4.0);
    ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
    ImPlot::PlotLine("P",
                     data(packet.plots.t),
                     data(packet.plots.p),
                     static_cast<int>(ssize(packet.plots.t)));
    ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
    ImPlot::PlotLine("N",
                     data(packet.plots.t),
                     data(packet.plots.n),
                     static_cast<int>(ssize(packet.plots.t)));

    // Highlights
    if (ImPlot::IsPlotHovered()) {
      ImPlotPoint const mouse{ImPlot::GetPlotMousePos()};

      if (auto const it{std::ranges::adjacent_find(
            packet.plots.t,
            [x = mouse.x](double a, double b) { return x >= a && x <= b; })};
          it != cend(packet.plots.t)) {
        auto const i{static_cast<size_t>(
          std::ranges::distance(cbegin(packet.plots.t), it) & ~3)};

        // Convert segment x coords to pixel space
        std::array const ps{
          ImPlot::PlotToPixels(ImPlotPoint{packet.plots.t[i], 0}),
          ImPlot::PlotToPixels(ImPlotPoint{packet.plots.t[i + 4uz], 0})};

        // Get plot rect (for full-height fill)
        ImVec2 const plot_pos{ImPlot::GetPlotPos()};
        ImVec2 const plot_size{ImPlot::GetPlotSize()};
        float const y_min{plot_pos.y};
        float const y_max{plot_pos.y + plot_size.y};

        ImDrawList* dl{ImPlot::GetPlotDrawList()};

        // Filled translucent rectangle covering the signal region
        auto const& [hgl_col, hgl_str]{packet.plots.highlights[i / 4uz]};
        dl->AddRectFilled(
          ImVec2{ps[0uz].x, y_min},
          ImVec2{ps[1uz].x, y_max},
          ImColor(
            hgl_col.Value.x, hgl_col.Value.y, hgl_col.Value.z, 50 / 255.0f));

        // Optional vertical line at the exact mouse x
        ImVec2 const px{ImPlot::PlotToPixels(ImPlotPoint{mouse.x, 0.0})};
        dl->AddLine(
          ImVec2{px.x, y_min},
          ImVec2{px.x, y_max},
          ImColor(
            hgl_col.Value.x, hgl_col.Value.y, hgl_col.Value.z, 150 / 255.0f),
          1.0f);

        ImGui::BeginTooltip();
        ImGui::TextUnformatted(hgl_str.c_str());
        ImGui::EndTooltip();
      }
    }

    // Tags
    for (auto const& [x, col, str] : packet.plots.tags)
      ImPlot::TagX(x, col, "%s", str.c_str());

    ImPlot::EndPlot();
  }
}

} // namespace tab

} // namespace

// Packets window
void packets() {
  if (!state.windows.show_packets) return;

  if (ImGui::Begin("Packets", &state.windows.show_packets)) {
    if (ImGui::BeginTabBar(UNIQUE_LABEL())) {
      size_t i{};

      for (auto& packet : state.packets) {
        eval::eval(packet);
        tab::tab(packet, i++);
      }

      ImGui::EndTabBar();
    }
  }
  ImGui::End();
}
