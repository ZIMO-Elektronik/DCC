#include "packets.hpp"
#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>
#include <bitset>
#include <format>
#include <numeric>
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
void eval(State& state, State::Packet& packet, bool service_mode);
  void preamble(State::Packet& packet);
  void address(State::Packet& packet, bool service_mode);
  void instruction(State::Packet& packet, bool service_mode);
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
      void advanced_operations_speed_direction_and_function(State::Packet& packet, std::span<uint8_t const> bytes);
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
    void cv_access(State::Packet& packet, std::span<uint8_t const> bytes, bool service_mode);
      void cv_access_long_form(State::Packet& packet, std::span<uint8_t const> bytes, bool service_mode);
      void cv_access_short_form(State::Packet& packet, std::span<uint8_t const> bytes);
      void cv_access_xpom(State::Packet& packet, std::span<uint8_t const> bytes);
    void digital_decoder_idle(State::Packet& packet, std::span<uint8_t const> bytes);
    void basic_accessory_decoder_control(State::Packet& packet, std::span<uint8_t const> bytes);
    void extended_accessory_decoder_control(State::Packet& packet, std::span<uint8_t const> bytes);
    void nop_for_basic_and_extended_accessory(State::Packet& packet, std::span<uint8_t const> bytes);
  void checksum(State::Packet& packet);
  void highlights(State::Packet& packet, bool service_mode);
  void tags(State::Packet& packet, bool service_mode);
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

//
void eval(State& state, State::Packet& packet, bool service_mode) {
  if (!empty(packet.desc_strs) && state.cfg == packet.cfg) return;

  //
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

  //
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
  address(packet, service_mode);
  instruction(packet, service_mode);
  checksum(packet);
  highlights(packet, service_mode);
  tags(packet, service_mode);
}

//
void preamble(State::Packet& packet) {
  packet.pattern_str += std::string(packet.cfg.num_preamble, 'S');
}

//
void address(State::Packet& packet, bool service_mode) {
  auto const addr{service_mode
                    ? dcc::Address{.type = dcc::Address::UnknownService}
                    : dcc::decode_address(packet.bytes)};
  switch (addr.type) {
    case dcc::Address::UnknownService:
      packet.desc_strs.push_back("Service");
      break;
    case dcc::Address::Broadcast:
      packet.desc_strs.push_back("Broadcast=" + std::to_string(addr));
      packet.pattern_str += " 0 00000000";
      break;
    case dcc::Address::BasicLoco:
      packet.desc_strs.push_back("Basic Loco=" + std::to_string(addr));
      packet.pattern_str += " 0 0AAAAAAA";
      break;
    case dcc::Address::BasicAccessory:
      packet.desc_strs.push_back("Basic Accessory=" + std::to_string(addr));
      packet.pattern_str += " 0 10AAAAAA 0 1ÂÂÂDAAR";
      break;
    case dcc::Address::ExtendedAccessory:
      packet.desc_strs.push_back("Extended Accessory=" + std::to_string(addr));
      packet.pattern_str += " 0 10AAAAAA 0 0ÂÂÂ0AA1";
      break;
    case dcc::Address::ExtendedLoco:
      packet.desc_strs.push_back("Extended Loco=" + std::to_string(addr));
      packet.pattern_str += " 0 11AAAAAA 0 AAAAAAAA";
      break;
    case dcc::Address::Reserved:
      packet.desc_strs.push_back("Reserved=" + std::to_string(addr));
      packet.pattern_str += " 0 11x1xxx";
      break;
    case dcc::Address::DataTransfer:
      packet.desc_strs.push_back("Data Transfer=" + std::to_string(addr));
      packet.pattern_str += " 0 11111101";
      break;
    case dcc::Address::AutomaticLogon:
      packet.desc_strs.push_back("Automatic Logon=" + std::to_string(addr));
      packet.pattern_str += " 0 11111110";
      break;
    case dcc::Address::Idle:
      packet.desc_strs.push_back("Idle=" + std::to_string(addr));
      packet.pattern_str += " 0 11111111";
      break;
  }
}

//
void instruction(State::Packet& packet, bool service_mode) {
  auto const addr{service_mode
                    ? dcc::Address{.type = dcc::Address::UnknownService}
                    : dcc::decode_address(packet.bytes)};
  auto first{cbegin(packet.bytes) +
             (packet.bytes[0uz] >= 128u && packet.bytes[0uz] <= 252u ? 2 : 1)};
  std::span<uint8_t const> bytes{first, cend(packet.bytes)};
  if (empty(bytes)) return;
  else if (addr.type == dcc::Address::UnknownService) {
    packet.desc_strs.push_back("CV Access");
    cv_access_long_form(
      packet, {cbegin(packet.bytes), cend(packet.bytes)}, service_mode);
  } else if (addr.type == dcc::Address::Broadcast ||
             addr.type == dcc::Address::BasicLoco ||
             addr.type == dcc::Address::ExtendedLoco)
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
      case dcc::Instruction::CvAccess:
        return cv_access(packet, bytes, service_mode);
      case dcc::Instruction::Logon: break;
    }
  else if (addr.type == dcc::Address::BasicAccessory)
    packet.bytes[1uz] & ztl::mask<7u>
      ? basic_accessory_decoder_control(packet, bytes)
      : nop_for_basic_and_extended_accessory(packet, bytes);
  else if (addr.type == dcc::Address::ExtendedAccessory)
    packet.bytes[1uz] & ztl::mask<3u>
      ? nop_for_basic_and_extended_accessory(packet, bytes)
      : extended_accessory_decoder_control(packet, bytes);
  else if (addr.type == dcc::Address::Idle) digital_decoder_idle(packet, bytes);
}

//
void unknown(State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.push_back("Unknown");
  for (auto i{0uz}; i < size(bytes) - 1uz; ++i)
    packet.pattern_str += " 0 ????????";
}

//
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

//
void decoder_control_digital_decoder_reset(State::Packet& packet,
                                           std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Digital Decoder Reset";
  packet.pattern_str += " 0 " + std::format("{:08b}", bytes[0uz]);
}

//
void decoder_control_hard_reset(State::Packet& packet,
                                std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Hard Reset";
  packet.pattern_str += " 0 " + std::format("{:08b}", bytes[0uz]);
}

//
void decoder_control_factory_test(State::Packet& packet,
                                  std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Factory Test";
  packet.pattern_str += " 0 0000000x";
  for (auto i{1uz}; i < size(bytes) - 1uz; ++i)
    packet.pattern_str += " 0 xxxxxxxx";
}

//
void decoder_control_set_advanced_addressing(State::Packet& packet,
                                             std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Set Advanced Addressing";
  packet.desc_strs.back() +=
    "\n- CV29:5=" + std::to_string(bytes[0uz] & ztl::mask<0u>);
  packet.pattern_str += " 0 0000101D";
}

//
void decoder_control_decoder_acknowledgement_request(
  State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Decoder Acknowledgement Request";
  packet.pattern_str += " 0 " + std::format("{:08b}", bytes[0uz]);
}

//
void consist_control(State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.push_back("Consist Control");
  switch (bytes[0uz]) {
    case 0b0001'0010: [[fallthrough]];
    case 0b0001'0011: return consist_control_set_consist_address(packet, bytes);
  }
}

//
void consist_control_set_consist_address(State::Packet& packet,
                                         std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Set Consist Address";
  packet.desc_strs.back() +=
    "\n- Reversed=" + std::to_string(bytes[0uz] & ztl::mask<0u>);
  packet.desc_strs.back() += "\n- Address=" + std::to_string(bytes[1uz]);
  packet.pattern_str += " 0 0001001R 0 0AAAAAAA";
}

//
void advanced_operations(State::Packet& packet,
                         std::span<uint8_t const> bytes) {
  packet.desc_strs.push_back("Advanced Operations");
  switch (bytes[0uz]) {
    case 0b0011'1100u:
      return advanced_operations_speed_direction_and_function(packet, bytes);
    case 0b0011'1101u:
      return advanced_operations_analog_function_group(packet, bytes);
    case 0b0011'1110u:
      return advanced_operations_special_operating_modes(packet, bytes);
    case 0b0011'1111u:
      return advanced_operations_128_speed_step_control(packet, bytes);
  }
}

//
void advanced_operations_speed_direction_and_function(
  State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Speed, Direction and Functions";
  packet.desc_strs.back() +=
    "\n- Direction=" +
    std::to_string(static_cast<bool>(bytes[1uz] & ztl::mask<7u>));
  packet.desc_strs.back() +=
    std::string{"\n- Speed="} +
    speed_labels[static_cast<size_t>(dcc::decode_rggggggg(bytes[1uz]) + 1)];
  packet.desc_strs.back() += "\n- F7-F0=" + std::format("{:08b}", bytes[2uz]);
  if (size(bytes) >= 4uz + 1uz)
    packet.desc_strs.back() +=
      "\n- F15-F8=" + std::format("{:08b}", bytes[3uz]);
  if (size(bytes) >= 5uz + 1uz)
    packet.desc_strs.back() +=
      "\n- F23-F16=" + std::format("{:08b}", bytes[4uz]);
  if (size(bytes) >= 6uz + 1uz)
    packet.desc_strs.back() +=
      "\n- F31-F24=" + std::format("{:08b}", bytes[4uz]);
  packet.pattern_str += " 0 00111100 0 RGGGGGGG 0 DDDDDDDD";
  for (auto i{3uz}; i < size(bytes) - 1uz; ++i)
    packet.pattern_str += " 0 DDDDDDDD";
}

//
void advanced_operations_analog_function_group(State::Packet& packet,
                                               std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Analog Function Group";
  packet.desc_strs.back() +=
    std::string{"\n- Channel="} + analog_labels[bytes[1uz]];
  packet.desc_strs.back() +=
    std::string{"\n- Value="} + std::to_string(bytes[2uz]);
  packet.pattern_str += " 0 00111101 0 SSSSSSSS 0 DDDDDDDD";
}

//
void advanced_operations_special_operating_modes(
  State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Special Operating Modes";
  auto const cc{(bytes[1uz] >> 2u) & 0b11u};
  packet.desc_strs.back() +=
    std::string{"\n- Consist="} +
    consist_labels[((cc & 1) << 1) | ((cc & 2) >> 1) | (cc & ~3)];
  packet.desc_strs.back() +=
    "\n- Shunting=" +
    std::to_string(static_cast<bool>(bytes[1uz] & ztl::mask<4u>));
  packet.desc_strs.back() +=
    "\n- West=" + std::to_string(static_cast<bool>(bytes[1uz] & ztl::mask<5u>));
  packet.desc_strs.back() +=
    "\n- East=" + std::to_string(static_cast<bool>(bytes[1uz] & ztl::mask<6u>));
  packet.desc_strs.back() +=
    "\n- MAN=" + std::to_string(static_cast<bool>(bytes[1uz] & ztl::mask<7u>));
  packet.pattern_str += " 0 00111110 0 DDDDDD00";
}

//
void advanced_operations_128_speed_step_control(
  State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - 128 Speed Step Control";
  packet.desc_strs.back() +=
    "\n- Direction=" +
    std::to_string(static_cast<bool>(bytes[1uz] & ztl::mask<7u>));
  packet.desc_strs.back() +=
    std::string{"\n- Speed="} +
    speed_labels[static_cast<size_t>(dcc::decode_rggggggg(bytes[1uz]) + 1)];
  packet.pattern_str += " 0 00111111 0 RGGGGGGG";
}

//
void speed_and_direction(State::Packet& packet,
                         std::span<uint8_t const> bytes) {
  packet.desc_strs.push_back("Speed & Direction");
  packet.desc_strs.back() +=
    "\n- Direction=" +
    std::to_string(static_cast<bool>(bytes[0uz] & ztl::mask<5u>));
  packet.desc_strs.back() +=
    std::string{"\n- Speed="} +
    speed_labels[static_cast<size_t>(dcc::decode_rggggg(bytes[0uz], true) + 1)];
  packet.desc_strs.back() +=
    std::string{" or Speed="} +
    speed_labels[static_cast<size_t>(dcc::decode_rggggg(bytes[0uz], false) +
                                     1)] +
    " and F0=" + std::to_string(static_cast<bool>(bytes[0uz] & ztl::mask<4u>));
  packet.pattern_str += " 0 01RGGGGG";
}

//
void function_group(State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.push_back("Function Group");
  switch (bytes[0uz] & 0xF0u) {
    case 0b1000'0000u: [[fallthrough]];
    case 0b1001'0000u: return function_group_f0_f4(packet, bytes);
    case 0b1010'0000u: return function_group_f9_f12(packet, bytes);
    case 0b1011'0000u: return function_group_f5_f8(packet, bytes);
  }
}

//
void function_group_f0_f4(State::Packet& packet,
                          std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - F0-F4";
  packet.desc_strs.back() +=
    "\n- F4-F0=" +
    std::format("{:05b}",
                (bytes[0uz] & 0x0Fu) << 1u | (bytes[0uz] & 0x1Fu) >> 4u);
  packet.pattern_str += " 0 100DDDDD";
}

//
void function_group_f9_f12(State::Packet& packet,
                           std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - F9-F12";
  packet.desc_strs.back() +=
    "\n- F12-F9=" + std::format("{:04b}", bytes[0uz] & 0x0Fu);
  packet.pattern_str += " 0 1010DDDD";
}

//
void function_group_f5_f8(State::Packet& packet,
                          std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - F5-F8";
  packet.desc_strs.back() +=
    "\n- F8-F5=" + std::format("{:04b}", bytes[0uz] & 0x0Fu);
  packet.pattern_str += " 0 1011DDDD";
}

//
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

//
void feature_expansion_binary_state_control_long_form(
  State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Binary State Control Long Form";
  packet.desc_strs.back() +=
    "\n- Address=" + std::to_string(bytes[2uz] << 7u | (bytes[1uz] & 0x7Fu));
  packet.desc_strs.back() +=
    "\n- State=" +
    std::to_string(static_cast<bool>(bytes[1uz] & ztl::mask<7u>));
  packet.pattern_str += " 0 11000000 0 DLLLLLLL 0 HHHHHHHH";
}

//
void feature_expansion_time_and_date(State::Packet& packet,
                                     std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Time and Date";
  switch (bytes[1uz] >> 6u) {
    case 0b00u:
      packet.desc_strs.back() += "\n- Minutes=" + std::to_string(bytes[1uz]);
      packet.desc_strs.back() +=
        std::string{"\n- Weekday="} + weekday_labels[bytes[2uz] >> 5u];
      packet.desc_strs.back() +=
        "\n- Hours=" + std::to_string(bytes[2uz] & 0x1Fu);
      packet.desc_strs.back() +=
        "\n- Update=" +
        std::to_string(static_cast<bool>(bytes[3uz] & ztl::mask<7u>));
      packet.desc_strs.back() +=
        "\n- Acceleration=" + std::to_string(bytes[3uz] & 0x3Fu);
      packet.pattern_str += " 0 11000001 0 00MMMMMM 0 WWWHHHHH 0 U0BBBBBB";
      break;
    case 0b01u:
      packet.desc_strs.back() +=
        "\n- Day=" + std::to_string(bytes[1uz] & 0x1Fu);
      packet.desc_strs.back() +=
        "\n- Month=" + std::to_string(bytes[2uz] >> 4u);
      packet.desc_strs.back() +=
        "\n- Year=" + std::to_string((bytes[2uz] & 0x0Fu) << 8u | bytes[3uz]);
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
        "\n- Time Scale=" + std::to_string(static_cast<float>(f16));
      packet.pattern_str += " 0 11000001 0 10111111 0 SEEEEEMM 0 MMMMMMMM";
      break;
  }
}

//
void feature_expansion_system_time(State::Packet& packet,
                                   std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - System Time";
  packet.desc_strs.back() +=
    "\n- Milliseconds=" + std::to_string(bytes[1uz] << 8u | bytes[2uz]);
  packet.pattern_str += " 0 11000010 0 MMMMMMMM 0 MMMMMMMM";
}

//
void feature_expansion_command_station_feature_identification(
  State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Command Station Feature Identification";
  switch (auto const iiii{bytes[1uz] & 0x0Fu}) {
    case 0b1111u: {
      dcc::LocoFeatures const feats{
        static_cast<uint16_t>(bytes[2uz] << 8u | bytes[3uz] << 0u)};
      packet.desc_strs.back() += "\n- Loco Features";
      packet.desc_strs.back() += "\n- Special Operating Modes=" +
                                 std::to_string(feats.special_operating_modes);
      packet.desc_strs.back() +=
        "\n- Analog Function=" + std::to_string(feats.analog_function);
      packet.desc_strs.back() +=
        "\n- Binary State Long=" + std::to_string(feats.binary_state_long);
      packet.desc_strs.back() +=
        "\n- Binary State Short=" + std::to_string(feats.binary_state_short);
      packet.desc_strs.back() += "\n- F29-F68=" + std::to_string(feats.f29_f68);
      packet.desc_strs.back() += "\n- F13-F28=" + std::to_string(feats.f13_f28);
      packet.desc_strs.back() +=
        "\n- XPOM Write=" + std::to_string(feats.xpom_write);
      packet.desc_strs.back() +=
        "\n- POM Write=" + std::to_string(feats.pom_write);
      packet.desc_strs.back() += "\n- Speed, Direction and Functions=" +
                                 std::to_string(feats.special_operating_modes);
      packet.desc_strs.back() +=
        "\n- 128 Speed Steps=" + std::to_string(feats.speed_steps_128);
      packet.desc_strs.back() +=
        "\n- Extended Addresses 10000-10239=" +
        std::to_string(feats.extended_addresses_10000_10239);
      packet.desc_strs.back() +=
        "\n- Basic Addresses 100-127 as Extended=" +
        std::to_string(feats.basic_addresses_100_127_as_extended);
      break;
    }
    case 0b1110u: {
      dcc::AccessoryBroadcastFeatures const feats{
        static_cast<uint16_t>(bytes[2uz] << 8u | bytes[3uz] << 0u)};
      packet.desc_strs.back() += "\n- Accessory and Broadcast Features";
      packet.desc_strs.back() +=
        "\n- System Time=" + std::to_string(feats.system_time);
      packet.desc_strs.back() +=
        "\n- Time Scale=" + std::to_string(feats.time_scale);
      packet.desc_strs.back() += "\n- Date=" + std::to_string(feats.date);
      packet.desc_strs.back() += "\n- Time=" + std::to_string(feats.time);
      packet.desc_strs.back() +=
        "\n- POM Write=" + std::to_string(feats.pom_write);
      packet.desc_strs.back() +=
        "\n- Extended=" + std::to_string(feats.extended);
      packet.desc_strs.back() += "\n- Addresses Offset by 4=" +
                                 std::to_string(feats.addresses_offset_by_4);
      break;
    }
    case 0b1101u: {
      dcc::BiDiFeatures const feats{
        static_cast<uint16_t>(bytes[2uz] << 8u | bytes[3uz] << 0u)};
      packet.desc_strs.back() += "\n- RailCom Features";
      packet.desc_strs.back() +=
        "\n- RailCom+=" + std::to_string(feats.railcom_plus);
      packet.desc_strs.back() += "\n- app:dyn Track Voltage=" +
                                 std::to_string(feats.app_dyn_track_voltage);
      packet.desc_strs.back() += "\n- app:dyn Operating Parameters=" +
                                 std::to_string(feats.app_dyn_operating_params);
      packet.desc_strs.back() += "\n- app:dyn Container Levels=" +
                                 std::to_string(feats.app_dyn_container_levels);
      packet.desc_strs.back() +=
        "\n- XPOM Read=" + std::to_string(feats.xpom_read);
      packet.desc_strs.back() +=
        "\n- POM Read=" + std::to_string(feats.pom_read);
      packet.desc_strs.back() +=
        "\n- NOP for Accessories=" + std::to_string(feats.nop_for_accessories);
      packet.desc_strs.back() += "\n- DCC-A=" + std::to_string(feats.dcc_a);
      packet.desc_strs.back() += "\n- RailCom=" + std::to_string(feats.railcom);
      break;
    }
  }
  packet.pattern_str += " 0 11000011 0 1111IIII 0 DDDDDDDD 0 DDDDDDDD";
}

//
void feature_expansion_f29_f36(State::Packet& packet,
                               std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - F29-F36";
  packet.desc_strs.back() += "\n- F36-F29=" + std::format("{:08b}", bytes[1uz]);
  packet.pattern_str += " 0 11011000 0 DDDDDDDD";
}

//
void feature_expansion_f37_f44(State::Packet& packet,
                               std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - F37-F44";
  packet.desc_strs.back() += "\n- F44-F37=" + std::format("{:08b}", bytes[1uz]);
  packet.pattern_str += " 0 11011001 0 DDDDDDDD";
}

//
void feature_expansion_f45_f52(State::Packet& packet,
                               std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - F45-F52";
  packet.desc_strs.back() += "\n- F52-F45=" + std::format("{:08b}", bytes[1uz]);
  packet.pattern_str += " 0 11011010 0 DDDDDDDD";
}

//
void feature_expansion_f53_f60(State::Packet& packet,
                               std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - F53-F60";
  packet.desc_strs.back() += "\n- F60-F53=" + std::format("{:08b}", bytes[1uz]);
  packet.pattern_str += " 0 11011011 0 DDDDDDDD";
}

//
void feature_expansion_f61_f68(State::Packet& packet,
                               std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - F61-F68";
  packet.desc_strs.back() += "\n- F68-F61=" + std::format("{:08b}", bytes[1uz]);
  packet.pattern_str += " 0 11011100 0 DDDDDDDD";
}

//
void feature_expansion_binary_state_control_short_form(
  State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Binary State Control Short Form";
  packet.desc_strs.back() +=
    "\n- Address=" + std::to_string(bytes[1uz] & 0x7Fu);
  packet.desc_strs.back() +=
    "\n- State=" +
    std::to_string(static_cast<bool>(bytes[1uz] & ztl::mask<7u>));
  packet.pattern_str += " 0 11011101 0 DLLLLLLL";
}

//
void feature_expansion_f13_f20(State::Packet& packet,
                               std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - F13-F20";
  packet.desc_strs.back() += "\n- F20-F13=" + std::format("{:08b}", bytes[1uz]);
  packet.pattern_str += " 0 11011110 0 DDDDDDDD";
}

//
void feature_expansion_f21_f28(State::Packet& packet,
                               std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - F21-F28";
  packet.desc_strs.back() += "\n- F28-F21=" + std::format("{:08b}", bytes[1uz]);
  packet.pattern_str += " 0 11011111 0 DDDDDDDD";
}

//
void cv_access(State::Packet& packet,
               std::span<uint8_t const> bytes,
               bool service_mode) {
  packet.desc_strs.push_back("CV Access");
  if (bytes[0uz] & ztl::mask<4u>) cv_access_short_form(packet, bytes);
  else if (size(bytes) <= 3uz + 1uz)
    cv_access_long_form(packet, bytes, service_mode);
  else cv_access_xpom(packet, bytes);
}

//
void cv_access_long_form(State::Packet& packet,
                         std::span<uint8_t const> bytes,
                         bool service_mode) {
  packet.desc_strs.back() += " - Long Form";
  auto const kk{bytes[0uz] >> 2u & 0b11u};
  auto const cv_addr{(bytes[0uz] & 0b11u) << 8u | bytes[1uz]};
  switch (kk) {
    case 0b01u: [[fallthrough]];
    case 0b11u:
      packet.desc_strs.back() +=
        kk == 0b01u ? "\n- Verify Byte" : "\n- Write Byte";
      packet.desc_strs.back() += "\n- CV" + std::to_string(cv_addr + 1u) + "=" +
                                 std::to_string(bytes[2uz]);
      packet.pattern_str += service_mode ? " 0 0111KKVV 0 VVVVVVVV 0 DDDDDDDD"
                                         : " 0 1110KKVV 0 VVVVVVVV 0 DDDDDDDD";
      break;
    case 0b10u:
      auto const pos{bytes[2uz] & 0b111u};
      auto const bit{static_cast<bool>(bytes[2uz] & ztl::mask<3u>)};
      packet.desc_strs.back() +=
        bytes[2uz] & ztl::mask<4u> ? "\n- Write Bit" : "\n- Verify Bit";
      packet.desc_strs.back() += "\n- CV" + std::to_string(cv_addr + 1u) + ":" +
                                 std::to_string(pos) + "=" +
                                 std::to_string(bit);
      packet.pattern_str += service_mode ? " 0 1110KKVV 0 VVVVVVVV 0 111KDBBB"
                                         : " 0 0111KKVV 0 VVVVVVVV 0 111KDBBB";
      break;
  }
}

//
void cv_access_short_form(State::Packet& packet,
                          std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - Short Form";
  auto const kkkk{bytes[0uz] & 0x0Fu};
  switch (kkkk) {
    case 0b0010u:
      packet.desc_strs.back() += "\n- Write Byte";
      packet.desc_strs.back() += "\n- CV23=" + std::to_string(bytes[1uz]);
      packet.pattern_str += " 0 1111KKKK 0 DDDDDDDD";
      break;
    case 0b0011u:
      packet.desc_strs.back() += "\n- Write Byte";
      packet.desc_strs.back() += "\n- CV24=" + std::to_string(bytes[1uz]);
      packet.pattern_str += " 0 1111KKKK 0 DDDDDDDD";
      break;
    case 0b0100u:
      packet.desc_strs.back() += "\n- Write Bytes";
      packet.desc_strs.back() += "\n- CV17=" + std::to_string(bytes[1uz]);
      packet.desc_strs.back() += "\n- CV18=" + std::to_string(bytes[2uz]);
      packet.pattern_str += " 0 1111KKKK 0 DDDDDDDD 0 DDDDDDDD";
      break;
    case 0b0101u:
      packet.desc_strs.back() += "\n- Write Bytes";
      packet.desc_strs.back() += "\n- CV31=" + std::to_string(bytes[1uz]);
      packet.desc_strs.back() += "\n- CV32=" + std::to_string(bytes[2uz]);
      packet.pattern_str += " 0 1111KKKK 0 DDDDDDDD 0 DDDDDDDD";
      break;
    case 0b0110u:
      packet.desc_strs.back() += "\n- Write Bytes";
      packet.desc_strs.back() += "\n- CV19=" + std::to_string(bytes[1uz]);
      packet.desc_strs.back() += "\n- CV20=" + std::to_string(bytes[2uz]);
      packet.pattern_str += " 0 1111KKKK 0 DDDDDDDD 0 DDDDDDDD";
      break;
  }
}

//
void cv_access_xpom(State::Packet& packet, std::span<uint8_t const> bytes) {
  packet.desc_strs.back() += " - XPOM";
  auto const kk{bytes[0uz] >> 2u & 0b11u};
  auto const ss{bytes[0uz] & 0b11u};
  auto const page_str{"Page CV31=" + std::to_string(bytes[1uz]) +
                      " CV32=" + std::to_string(bytes[2uz])};
  packet.desc_strs.back() += "\n- Sequence Number=" + std::format("{:02b}", ss);
  switch (kk) {
    case 0b01u:
      packet.desc_strs.back() += "\n- Verify Bytes @ " + page_str;
      packet.desc_strs.back() += "\n- CV" + std::to_string(bytes[3uz] + 1u);
      packet.desc_strs.back() += "\n- CV" + std::to_string(bytes[3uz] + 2u);
      packet.desc_strs.back() += "\n- CV" + std::to_string(bytes[3uz] + 3u);
      packet.desc_strs.back() += "\n- CV" + std::to_string(bytes[3uz] + 4u);
      packet.pattern_str += " 0 1110KKSS 0 VVVVVVVV 0 VVVVVVVV 0 VVVVVVVV";
      break;
    case 0b11u:
      packet.desc_strs.back() += "\n- Write Bytes @ " + page_str;
      packet.pattern_str += " 0 1110KKSS 0 VVVVVVVV 0 VVVVVVVV 0 VVVVVVVV";
      for (auto i{4uz}; i < size(bytes) - 1uz; ++i) {
        packet.desc_strs.back() += "\n- CV" +
                                   std::to_string(bytes[3uz] + 1u + i - 4u) +
                                   "=" + std::to_string(bytes[i]);
        packet.pattern_str += " 0 DDDDDDDD";
      }
      break;
    case 0b10u:
      auto const pos{bytes[4uz] & 0b111u};
      auto const bit{static_cast<bool>(bytes[4uz] & ztl::mask<3u>)};
      packet.desc_strs.back() += "\n- Write Bit @ " + page_str;
      packet.desc_strs.back() += "\n- CV" + std::to_string(bytes[3uz] + 1u) +
                                 ":" + std::to_string(pos) + "=" +
                                 std::to_string(bit);
      packet.pattern_str +=
        " 0 1110KKSS 0 VVVVVVVV 0 VVVVVVVV 0 VVVVVVVV 0 1111DBBB";
      break;
  }
}

//
void digital_decoder_idle(State::Packet& packet,
                          std::span<uint8_t const> bytes) {
  packet.desc_strs.push_back("Digital Decoder Idle");
  packet.pattern_str += " 0 00000000";
}

//
void basic_accessory_decoder_control(State::Packet& packet,
                                     std::span<uint8_t const>) {
  packet.desc_strs.push_back("Basic Accessory Decoder Control");
  packet.desc_strs.back() +=
    "\n- Output=" +
    std::to_string(static_cast<bool>(packet.bytes[1uz] & ztl::mask<0u>));
  packet.desc_strs.back() +=
    "\n- State=" +
    std::to_string(static_cast<bool>(packet.bytes[1uz] & ztl::mask<3u>));
}

//
void extended_accessory_decoder_control(State::Packet& packet,
                                        std::span<uint8_t const> bytes) {
  packet.desc_strs.push_back("Extended Accessory Decoder Control");
  packet.desc_strs.back() +=
    "\n- State=" + std::format("{:08b}", bytes[0uz]) + " or";
  auto const switch_on_time_label{switch_on_time_labels[bytes[0uz] & 0x7Fu]};
  packet.desc_strs.back() +=
    "\n- Output=" +
    std::to_string(static_cast<bool>(bytes[0uz] & ztl::mask<7u>)) +
    +" for Time=" + switch_on_time_label;
  if (std::string_view{switch_on_time_label}.contains('.'))
    packet.desc_strs.back() += 's';
  packet.pattern_str += " 0 DDDDDDDD";
}

//
void nop_for_basic_and_extended_accessory(State::Packet& packet,
                                          std::span<uint8_t const> bytes) {
  packet.desc_strs.push_back("NOP for Basic and Extended Accessory Decoder");
}

//
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

//
void highlights(State::Packet& packet, bool service_mode) {
  auto const addr{dcc::decode_address(packet.bytes)};
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

  // Carot index
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
  if (!service_mode)
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

//
void tags(State::Packet& packet, bool service_mode) {
  auto const addr{service_mode
                    ? dcc::Address{.type = dcc::Address::UnknownService}
                    : dcc::decode_address(packet.bytes)};
  auto first{cbegin(packet.bytes) +
             (packet.bytes[0uz] >= 128u && packet.bytes[0uz] <= 252u ? 2 : 1)};
  std::span<uint8_t const> bytes{first, cend(packet.bytes)};

  auto const num_preamble{packet.cfg.num_preamble};
  auto const bit1_duration{packet.cfg.bit1_duration};
  auto const bit0_duration{packet.cfg.bit0_duration};

  double t{0.0};
  packet.plots.tags.push_back({t, PRE_COL, "Preamble"});
  t += 2.0 * (num_preamble * bit1_duration + bit0_duration);

  if (!service_mode) {
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

//
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

//
void description(State::Packet& packet) {
  for (auto const& desc : packet.desc_strs)
    ImGui::BulletText("%s", desc.c_str());
}

//
void data(State::Packet& packet) {
  ImGui::BinaryTable(UNIQUE_LABEL(),
                     data(packet.bytes),
                     ssize(packet.bytes),
                     ImGuiInputTextFlags_ReadOnly);
}

//
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

//
void packets(State& state) {
  if (!state.windows.show_packets) return;

  if (ImGui::Begin("Packets", &state.windows.show_packets)) {
    if (ImGui::BeginTabBar(UNIQUE_LABEL())) {
      size_t i{};

      for (auto& packet : state.operations_packets) {
        eval::eval(state, packet, false);
        tab::tab(packet, i++);
      }

      for (auto& packet : state.service_packets) {
        eval::eval(state, packet, true);
        tab::tab(packet, i++);
      }

      ImGui::EndTabBar();
    }
  }
  ImGui::End();
}
