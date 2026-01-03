#include "packet_builder.hpp"
#include <imgui.h>
#include <implot.h>
#include "state.hpp"
#include "utility.hpp"

namespace {

namespace broadcast {

// clang-format off
void broadcast();
  void decoder_control(dcc::Address addr);
    void decoder_control_digital_decoder_reset(dcc::Address addr);
  void speed_and_direction(dcc::Address addr);
  void feature_expansion(dcc::Address addr);
    void feature_expansion_time_and_date(dcc::Address addr);
    void feature_expansion_system_time(dcc::Address addr);
    void feature_expansion_command_station_feature_identification(dcc::Address addr);
// clang-format on

} // namespace broadcast

namespace loco {

// clang-format off
void basic();
void extended();
  void loco(dcc::Address addr);
    void decoder_control(dcc::Address addr);
      void decoder_control_digital_decoder_reset(dcc::Address addr);
      void decoder_control_hard_reset(dcc::Address addr);
      void decoder_control_factory_test(dcc::Address addr);
      void decoder_control_set_advanced_addressing(dcc::Address addr);
      void decoder_control_decoder_acknowledgement_request(dcc::Address addr);
    void consist_control(dcc::Address addr);
      void consist_control_set_consist_address(dcc::Address addr);
    void advanced_operations(dcc::Address addr);
      void advanced_operations_speed_direction_and_functions(dcc::Address addr);
      void advanced_operations_analog_function_group(dcc::Address addr);
      void advanced_operations_special_operating_modes(dcc::Address addr);
      void advanced_operations_128_speed_step_control(dcc::Address addr);
    void speed_and_direction(dcc::Address addr);
    void function_group(dcc::Address addr);
      void function_group_f0_f4(dcc::Address addr);
      void function_group_f9_f12(dcc::Address addr);
      void function_group_f5_f8(dcc::Address addr);
    void feature_expansion(dcc::Address addr);
      void feature_expansion_binary_state_control_long_form(dcc::Address addr);
      void feature_expansion_f29_f36(dcc::Address addr);
      void feature_expansion_f37_f44(dcc::Address addr);
      void feature_expansion_f45_f52(dcc::Address addr);
      void feature_expansion_f53_f60(dcc::Address addr);
      void feature_expansion_f61_f68(dcc::Address addr);
      void feature_expansion_binary_state_control_short_form(dcc::Address addr);
      void feature_expansion_f13_f20(dcc::Address addr);
      void feature_expansion_f21_f28(dcc::Address addr);
    void cv_access(dcc::Address addr);
      void cv_access_long_form(dcc::Address addr);
      void cv_access_short_form(dcc::Address addr);
      void cv_access_xpom(dcc::Address addr);
// clang-format on

} // namespace loco

namespace accessory {

// clang-format off
void basic();
void extended();
  void accessory(dcc::Address addr);
    void basic_accessory_decoder_control(dcc::Address addr);
    void extended_accessory_decoder_control(dcc::Address addr);
    void nop_for_basic_and_extended_accessory(dcc::Address addr);
// clang-format on

} // namespace accessory

namespace automatic_logon {

// clang-format off
void automatic_logon();
// clang-format on

} // namespace automatic_logon

namespace idle {

// clang-format off
void idle();
  void digital_decoder_idle(dcc::Address addr);
// clang-format on

} // namespace idle

namespace broadcast {

// Broadcast
void broadcast() {
  ImGui::SeparatorText("Instruction");
  static constexpr std::array instrs{
    "", "Decoder Control", "Speed & Direction", "Feature Expansion"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
  if (!strcmp(instrs[static_cast<size_t>(i)], "Decoder Control"))
    decoder_control({.type = dcc::Address::Broadcast});
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Speed & Direction"))
    speed_and_direction({.type = dcc::Address::Broadcast});
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Feature Expansion"))
    feature_expansion({.type = dcc::Address::Broadcast});
}

// Broadcast decoder control
void decoder_control(dcc::Address addr) {
  ImGui::SeparatorText("Sub Instruction");
  static constexpr std::array instrs{"", "Digital Decoder Reset"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
  if (!strcmp(instrs[static_cast<size_t>(i)], "Digital Decoder Reset"))
    decoder_control_digital_decoder_reset(addr);
}

// Broadcast decoder control - digital decoder reset
void decoder_control_digital_decoder_reset(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  ImGui::Text("None");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back({.addr = addr, .bytes = dcc::make_reset_packet()});
}

// Broadcast speed and direction
void speed_and_direction(dcc::Address addr) {
  ImGui::BeginDisabled();
  ImGui::SeparatorText("Sub Instruction");
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, "", 1);
  ImGui::EndDisabled();
  ImGui::SeparatorText("Parameters");
  static uint8_t rggggg{ztl::mask<5u, 4u>};
  auto r{static_cast<bool>(rggggg & ztl::mask<5u>)};
  ImGui::Checkbox("Direction", &r);
  ImGui::SameLine();
  auto c{static_cast<bool>(rggggg & ztl::mask<4u>)};
  ImGui::Checkbox("Ignore Direction", &c);
  static constexpr int8_t const min{-1};
  static constexpr int8_t const max{0};
  static int8_t speed{};
  ImGui::SliderScalar("Speed",
                      ImGuiDataType_S8,
                      &speed,
                      &min,
                      &max,
                      speed_labels[static_cast<size_t>(speed + 1)]);
  rggggg = dcc::encode_rggggg(r, speed) | c << 4u;
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr,
       .bytes = dcc::make_speed_and_direction_packet(addr, rggggg)});
}

// Broadcast feature expansion
void feature_expansion(dcc::Address addr) {
  ImGui::SeparatorText("Sub Instruction");
  static constexpr std::array instrs{"",
                                     "Binary State Control Long Form",
                                     "Time and Date",
                                     "System Time",
                                     "Command Station Feature Identification",
                                     "Binary State Control Short Form"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
  if (!strcmp(instrs[static_cast<size_t>(i)], "Binary State Control Long Form"))
    ::loco::feature_expansion_binary_state_control_long_form(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Time and Date"))
    feature_expansion_time_and_date(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "System Time"))
    feature_expansion_system_time(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)],
                   "Command Station Feature Identification"))
    feature_expansion_command_station_feature_identification(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)],
                   "Binary State Control Short Form"))
    ::loco::feature_expansion_binary_state_control_short_form(addr);
}

// Broadcast feature expansion - time and date
void feature_expansion_time_and_date(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static constexpr std::array chrono_types{"", "Time", "Date", "Time Scale"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(chrono_types), ssize(chrono_types));
  if (!strcmp(chrono_types[static_cast<size_t>(i)], "Time")) {
    static std::array<uint8_t, 2uz> time{};
    ImGui::InputScalarN(
      "[hh::mm]", ImGuiDataType_U8, data(time), 2, nullptr, nullptr, "%02u");
    time[0uz] = std::clamp<uint8_t>(time[0uz], 0u, 23u);
    time[1uz] = std::clamp<uint8_t>(time[1uz], 0u, 59u);
    static constexpr uint8_t weekday_min{0u};
    static constexpr uint8_t weekday_max{7u};
    static uint8_t weekday{};
    ImGui::SliderScalar("Weekday",
                        ImGuiDataType_U8,
                        &weekday,
                        &weekday_min,
                        &weekday_max,
                        weekday_labels[weekday]);
    static constexpr uint8_t acc_min{0u};
    static constexpr uint8_t acc_max{63u};
    static uint8_t acc{1};
    ImGui::SliderScalar(
      "Acceleration", ImGuiDataType_U8, &acc, &acc_min, &acc_max, "%u");
    static bool update{};
    ImGui::Checkbox("Update", &update);
    ImGui::SeparatorText("Done");
    if (ImGui::Button("Push to Packets"))
      state.packets.push_back({.addr = addr,
                               .bytes = dcc::make_time_packet(
                                 weekday, time[0uz], time[1uz], acc, update)});
  } else if (!strcmp(chrono_types[static_cast<size_t>(i)], "Date")) {
    static std::array<uint16_t, 3uz> date{24u, 3u, 1989u};
    ImGui::InputScalarN("[dd::mm:yyyy]",
                        ImGuiDataType_U16,
                        data(date),
                        ssize(date),
                        nullptr,
                        nullptr,
                        "%02u");
    date[0uz] = std::clamp<uint16_t>(date[0uz], 1u, 31u);
    date[1uz] = std::clamp<uint16_t>(date[1uz], 1u, 12u);
    date[2uz] = std::clamp<uint16_t>(date[2uz], 0u, 4095u);
    ImGui::SeparatorText("Done");
    if (ImGui::Button("Push to Packets"))
      state.packets.push_back(
        {.addr = addr,
         .bytes = dcc::make_date_packet(static_cast<uint8_t>(date[0uz]),
                                        static_cast<uint8_t>(date[1uz]),
                                        date[2uz])});
  } else if (!strcmp(chrono_types[static_cast<size_t>(i)], "Time Scale")) {
    static float scale{};
    ImGui::InputFloat("Time Scale", &scale);
    ImGui::SeparatorText("Done");
    if (ImGui::Button("Push to Packets"))
      state.packets.push_back(
        {.addr = addr,
         .bytes = dcc::make_time_scale_packet(static_cast<FLOAT16>(scale))});
  }
}

// Broadcast feature expansion - system time
void feature_expansion_system_time(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static uint16_t ms{};
  ImGui::InputScalar("[ms]", ImGuiDataType_U16, &ms);
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr, .bytes = dcc::make_system_time_packet(ms)});
}

// Broadcast feature expansion - command station feature identification
void feature_expansion_command_station_feature_identification(
  dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static constexpr std::array feat_types{
    "", "Loco Features", "Accessory and Broadcast Features", "BiDi Features"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(feat_types), ssize(feat_types));
  if (!strcmp(feat_types[static_cast<size_t>(i)], "Loco Features")) {
    static std::array<bool, CHAR_BIT * sizeof(uint16_t)> feats{};
    ImGui::Checkbox("Basic Addresses 100-127 as Extended", &feats[0uz]);
    ImGui::Checkbox("Extended Addresses 10000-10239", &feats[1uz]);
    ImGui::Checkbox("128 Speed Steps", &feats[2uz]);
    ImGui::Checkbox("Speed, Direction and Functions", &feats[3uz]);
    ImGui::Checkbox("POM Write", &feats[4uz]);
    ImGui::Checkbox("XPOM Write", &feats[5uz]);
    ImGui::Checkbox("F13-F28", &feats[8uz]);
    ImGui::Checkbox("F29-F68", &feats[9uz]);
    ImGui::Checkbox("Binary State Short", &feats[10uz]);
    ImGui::Checkbox("Binary State Long", &feats[11uz]);
    ImGui::Checkbox("Analog Function", &feats[12uz]);
    ImGui::Checkbox("Special Operating Modes", &feats[13uz]);
    ImGui::SeparatorText("Done");
    if (ImGui::Button("Push to Packets"))
      state.packets.push_back(
        {.addr = addr,
         .bytes = dcc::make_command_station_feature_identification_packet(
           dcc::LocoFeatures{static_cast<uint16_t>(
             feats[13uz] << 13u | feats[12uz] << 12u | feats[11uz] << 11u |
             feats[10uz] << 10u | feats[9uz] << 9u | feats[8uz] << 8u |
             feats[5uz] << 5u | feats[4uz] << 4u | feats[3uz] << 3u |
             feats[2uz] << 2u | feats[1uz] << 1u | feats[0uz] << 0u)})});
  } else if (!strcmp(feat_types[static_cast<size_t>(i)],
                     "Accessory and Broadcast Features")) {
    static std::array<bool, CHAR_BIT * sizeof(uint16_t)> feats{};
    ImGui::Checkbox("Addresses Offset by 4", &feats[0uz]);
    ImGui::Checkbox("Extended", &feats[1uz]);
    ImGui::Checkbox("POM Write", &feats[3uz]);
    ImGui::Checkbox("Time", &feats[8uz]);
    ImGui::Checkbox("Date", &feats[9uz]);
    ImGui::Checkbox("Time Scale", &feats[10uz]);
    ImGui::Checkbox("System Time", &feats[11uz]);
    ImGui::SeparatorText("Done");
    if (ImGui::Button("Push to Packets"))
      state.packets.push_back(
        {.addr = addr,
         .bytes = dcc::make_command_station_feature_identification_packet(
           dcc::AccessoryBroadcastFeatures{static_cast<uint16_t>(
             feats[11uz] << 11u | feats[10uz] << 10u | feats[9uz] << 9u |
             feats[8uz] << 8u | feats[3uz] << 3u | feats[1uz] << 1u |
             feats[0uz] << 0u)})});
  } else if (!strcmp(feat_types[static_cast<size_t>(i)], "BiDi Features")) {
    static std::array<bool, CHAR_BIT * sizeof(uint16_t)> feats{};
    ImGui::Checkbox("RailCom", &feats[0uz]);
    ImGui::Checkbox("DCC-A", &feats[1uz]);
    ImGui::Checkbox("NOP for Accessories", &feats[2uz]);
    ImGui::Checkbox("POM Read", &feats[3uz]);
    ImGui::Checkbox("XPOM Read", &feats[4uz]);
    ImGui::Checkbox("app:dyn Container Levels", &feats[8uz]);
    ImGui::Checkbox("app:dyn Operating Parameters", &feats[9uz]);
    ImGui::Checkbox("app:dyn Track Voltage", &feats[10uz]);
    ImGui::Checkbox("RailCom+", &feats[15uz]);
    ImGui::SeparatorText("Done");
    if (ImGui::Button("Push to Packets"))
      state.packets.push_back(
        {.addr = addr,
         .bytes = dcc::make_command_station_feature_identification_packet(
           dcc::BiDiFeatures{static_cast<uint16_t>(
             feats[15uz] << 15u | feats[10uz] << 10u | feats[9uz] << 9u |
             feats[8uz] << 8u | feats[4uz] << 4u | feats[3uz] << 3u |
             feats[2uz] << 2u | feats[1uz] << 1u | feats[0uz] << 0u)})});
  }
}

} // namespace broadcast

////////////////////////////////////////////////////////////////////////////////

namespace loco {

// Basic loco
void basic() {
  ImGui::SeparatorText("Address");
  static dcc::Address::value_type addr{3};
  ImGui::InputScalar(UNIQUE_LABEL(), ImGuiDataType_U16, &addr);
  addr = std::clamp<dcc::Address::value_type>(addr, 1u, 127u);
  loco({.value = addr, .type = dcc::Address::BasicLoco});
}

// Extended loco
void extended() {
  ImGui::SeparatorText("Address");
  static dcc::Address::value_type addr{3};
  ImGui::InputScalar(UNIQUE_LABEL(), ImGuiDataType_U16, &addr);
  addr = std::clamp<dcc::Address::value_type>(addr, 1u, 10239u);
  loco({.value = addr, .type = dcc::Address::ExtendedLoco});
}

// Loco
void loco(dcc::Address addr) {
  ImGui::SeparatorText("Instruction");
  static constexpr std::array instrs{"",
                                     "Decoder Control",
                                     "Consist Control",
                                     "Advanced Operations",
                                     "Speed & Direction",
                                     "Function Group",
                                     "Feature Expansion",
                                     "CV Access"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
  if (!strcmp(instrs[static_cast<size_t>(i)], "Decoder Control"))
    decoder_control(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Consist Control"))
    consist_control(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Advanced Operations"))
    advanced_operations(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Speed & Direction"))
    speed_and_direction(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Function Group"))
    function_group(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Feature Expansion"))
    feature_expansion(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "CV Access"))
    cv_access(addr);
}

// Loco decoder control
void decoder_control(dcc::Address addr) {
  ImGui::SeparatorText("Sub Instruction");
  static constexpr std::array instrs{"",
                                     "Digital Decoder Reset",
                                     "Hard Reset",
                                     "Factory Test",
                                     "Set Advanced Addressing",
                                     "Decoder Acknowledgement Request"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
  if (!strcmp(instrs[static_cast<size_t>(i)], "Digital Decoder Reset"))
    decoder_control_digital_decoder_reset(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Hard Reset"))
    decoder_control_hard_reset(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Factory Test"))
    decoder_control_factory_test(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Set Advanced Addressing"))
    decoder_control_set_advanced_addressing(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)],
                   "Decoder Acknowledgement Request"))
    decoder_control_decoder_acknowledgement_request(addr);
}

// Loco decoder control - digital decoder reset
void decoder_control_digital_decoder_reset(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  ImGui::Text("None");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr, .bytes = dcc::make_reset_packet(addr)});
}

// Loco decoder control - hard reset
void decoder_control_hard_reset(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  ImGui::Text("None");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr, .bytes = dcc::make_hard_reset_packet(addr)});
}

// Loco decoder control - factory test
void decoder_control_factory_test(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static bool bit0{};
  ImGui::Checkbox("Bit0", &bit0);
  static std::vector<uint8_t> bytes{};
  if (ImGui::Button("-") && size(bytes)) bytes.pop_back();
  ImGui::SameLine();
  if (ImGui::Button("+") && size(bytes) < DCC_MAX_PACKET_SIZE)
    bytes.push_back(0u);
  ImGui::SameLine();
  ImGui::Text("Length");
  ImGui::Separator();
  ImGui::BinaryTable(
    UNIQUE_LABEL(), data(bytes), static_cast<int>(ssize(bytes)));
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr,
       .bytes = dcc::make_factory_test_packet(addr, bit0, bytes)});
}

// Loco decoder control - set advanced addressing
void decoder_control_set_advanced_addressing(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static bool cv29_5{};
  ImGui::Checkbox("CV29:5", &cv29_5);
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr,
       .bytes = dcc::make_set_advanced_addressing_packet(addr, cv29_5)});
}

// Loco decoder control - decoder acknowledgement request
void decoder_control_decoder_acknowledgement_request(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  ImGui::Text("None");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Decoder Acknowledgement Request"))
    state.packets.push_back(
      {.addr = addr, .bytes = dcc::make_ack_request_packet(addr)});
}

// Loco consist control
void consist_control(dcc::Address addr) {
  ImGui::SeparatorText("Sub Instruction");
  static constexpr std::array instrs{"", "Set Consist Address"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
  if (!strcmp(instrs[static_cast<size_t>(i)], "Set Consist Address"))
    consist_control_set_consist_address(addr);
}

// Loco consist control - set consist address
void consist_control_set_consist_address(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static dcc::Address::value_type consist_addr{3};
  ImGui::InputScalar("Address", ImGuiDataType_U8, &consist_addr);
  consist_addr = std::clamp<dcc::Address::value_type>(consist_addr, 1u, 127u);
  static bool r{};
  ImGui::Checkbox("Reversed", &r);
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back({.addr = addr,
                             .bytes = dcc::make_set_consist_address_packet(
                               addr, r << 7u | consist_addr)});
}

// Loco advanced operations
void advanced_operations(dcc::Address addr) {
  ImGui::SeparatorText("Sub Instruction");
  static constexpr std::array instrs{"",
                                     "Speed, Direction and Functions",
                                     "Analog Function Group",
                                     "Special Operating Modes",
                                     "128 Speed Step Control"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
  if (!strcmp(instrs[static_cast<size_t>(i)], "Speed, Direction and Functions"))
    advanced_operations_speed_direction_and_functions(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Analog Function Group"))
    advanced_operations_analog_function_group(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Special Operating Modes"))
    advanced_operations_special_operating_modes(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "128 Speed Step Control"))
    advanced_operations_128_speed_step_control(addr);
}

// Loco advanced operations - speed, direction and functions
void advanced_operations_speed_direction_and_functions(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");

  // Speed and direction
  static uint8_t rggggggg{ztl::mask<7u>};
  auto r{static_cast<bool>(rggggggg & ztl::mask<7u>)};
  ImGui::Checkbox("Direction", &r);
  static constexpr int8_t const min{-1};
  static constexpr int8_t const max{126};
  static int8_t speed{};
  ImGui::SliderScalar("Speed",
                      ImGuiDataType_S8,
                      &speed,
                      &min,
                      &max,
                      speed_labels[static_cast<size_t>(speed + 1)]);
  rggggggg = dcc::encode_rggggggg(r, speed);

  // Functions
  static size_t length{8uz};
  static std::array<bool, 32uz> d{};
  if (ImGui::Button("-") && length > 8uz) length -= 8uz;
  ImGui::SameLine();
  if (ImGui::Button("+") && length < d.max_size()) length += 8uz;
  ImGui::SameLine();
  ImGui::Text("Length");
  for (auto i{8uz}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("F7-F0");
  if (length > 8uz) {
    for (auto i{16uz}; i-- > 8uz;) {
      ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
      ImGui::SameLine();
    }
    ImGui::TextUnformatted("F15-F8");
  }
  if (length > 16uz) {
    for (auto i{24uz}; i-- > 16uz;) {
      ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
      ImGui::SameLine();
    }
    ImGui::TextUnformatted("F23-F16");
  }
  if (length > 24uz) {
    for (auto i{32uz}; i-- > 24uz;) {
      ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
      ImGui::SameLine();
    }
    ImGui::TextUnformatted("F31-F24");
  }

  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets")) {
    std::array<uint8_t, 4uz> const fs{
      static_cast<uint8_t>(d[0uz + 7uz] << 7u | d[0uz + 6uz] << 6u |
                           d[0uz + 5uz] << 5u | d[0uz + 4uz] << 4u |
                           d[0uz + 3uz] << 3u | d[0uz + 2uz] << 2u |
                           d[0uz + 1uz] << 1u | d[0uz + 0uz] << 0u),
      static_cast<uint8_t>(d[8uz + 7uz] << 7u | d[8uz + 6uz] << 6u |
                           d[8uz + 5uz] << 5u | d[8uz + 4uz] << 4u |
                           d[8uz + 3uz] << 3u | d[8uz + 2uz] << 2u |
                           d[8uz + 1uz] << 1u | d[8uz + 0uz] << 0u),
      static_cast<uint8_t>(d[16uz + 7uz] << 7u | d[16uz + 6uz] << 6u |
                           d[16uz + 5uz] << 5u | d[16uz + 4uz] << 4u |
                           d[16uz + 3uz] << 3u | d[16uz + 2uz] << 2u |
                           d[16uz + 1uz] << 1u | d[16uz + 0uz] << 0u),
      static_cast<uint8_t>(d[24uz + 7uz] << 7u | d[24uz + 6uz] << 6u |
                           d[24uz + 5uz] << 5u | d[24uz + 4uz] << 4u |
                           d[24uz + 3uz] << 3u | d[24uz + 2uz] << 2u |
                           d[24uz + 1uz] << 1u | d[24uz + 0uz] << 0u)};
    switch (length) {
      case 8uz:
        state.packets.push_back(
          {.addr = addr,
           .bytes = make_speed_direction_and_functions_packet(
             addr, rggggggg, fs[0uz])});
        break;
      case 16uz:
        state.packets.push_back(
          {.addr = addr,
           .bytes = make_speed_direction_and_functions_packet(
             addr, rggggggg, fs[0uz], fs[1uz])});
        break;
      case 24uz:
        state.packets.push_back(
          {.addr = addr,
           .bytes = make_speed_direction_and_functions_packet(
             addr, rggggggg, fs[0uz], fs[1uz], fs[2uz])});
        break;
      case 32uz:
        state.packets.push_back(
          {.addr = addr,
           .bytes = make_speed_direction_and_functions_packet(
             addr, rggggggg, fs[0uz], fs[1uz], fs[2uz], fs[3uz])});
        break;
    }
  }
}

// Loco advanced operations - analog function group
void advanced_operations_analog_function_group(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static int ssssssss{};
  ImGui::Combo("Channel", &ssssssss, data(analog_labels), ssize(analog_labels));
  static uint8_t dddddddd{};
  ImGui::InputScalar("Value", ImGuiDataType_U8, &dddddddd);
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr,
       .bytes = dcc::make_analog_function_group_packet(
         addr, static_cast<uint8_t>(ssssssss), dddddddd)});
}

// Loco advanced operations - special operating modes
void advanced_operations_special_operating_modes(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static int i{};
  ImGui::Combo("Consist", &i, data(consist_labels), ssize(consist_labels));
  static bool shunting{};
  ImGui::Checkbox("Shunting", &shunting);
  static bool west{};
  ImGui::Checkbox("West", &west);
  static bool east{};
  ImGui::Checkbox("East", &east);
  static bool man{};
  ImGui::Checkbox("MAN", &man);
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr,
       .bytes = dcc::make_special_operating_modes(
         addr,
         static_cast<dcc::Consist>(((i & 1) << 1) | ((i & 2) >> 1) | (i & ~3)),
         shunting,
         west,
         east,
         man)});
}

// Loco advanced operations - 128 speed step control
void advanced_operations_128_speed_step_control(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static uint8_t rggggggg{ztl::mask<7u>};
  auto r{static_cast<bool>(rggggggg & ztl::mask<7u>)};
  ImGui::Checkbox("Direction", &r);
  static constexpr int8_t const min{-1};
  static constexpr int8_t const max{126};
  static int8_t speed{};
  ImGui::SliderScalar("Speed",
                      ImGuiDataType_S8,
                      &speed,
                      &min,
                      &max,
                      speed_labels[static_cast<size_t>(speed + 1)]);
  rggggggg = dcc::encode_rggggggg(r, speed);
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr,
       .bytes = dcc::make_128_speed_step_control_packet(addr, rggggggg)});
}

// Loco speed and direction
void speed_and_direction(dcc::Address addr) {
  ImGui::BeginDisabled();
  ImGui::SeparatorText("Sub Instruction");
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, "", 1);
  ImGui::EndDisabled();
  ImGui::SeparatorText("Parameters");
  static uint8_t rggggg{ztl::mask<5u>};
  auto r{static_cast<bool>(rggggg & ztl::mask<5u>)};
  ImGui::Checkbox("Direction", &r);
  ImGui::SameLine();
  static bool cv29_1{true};
  ImGui::Checkbox("CV29:1", &cv29_1);
  static constexpr int8_t const min{-1};
  auto const max{static_cast<int8_t>(cv29_1 ? 28 : 14)};
  static int8_t speed{};
  speed = std::clamp(speed, min, max);
  ImGui::SliderScalar("Speed",
                      ImGuiDataType_S8,
                      &speed,
                      &min,
                      &max,
                      speed_labels[static_cast<size_t>(speed + 1)]);
  rggggg = dcc::encode_rggggg(r, speed, cv29_1);
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr,
       .bytes = dcc::make_speed_and_direction_packet(addr, rggggg)});
}

// Loco function group
void function_group(dcc::Address addr) {
  ImGui::SeparatorText("Sub Instruction");
  static constexpr std::array instrs{"", "F0-F4", "F9-F12", "F5-F8"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
  if (!strcmp(instrs[static_cast<size_t>(i)], "F0-F4"))
    function_group_f0_f4(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "F9-F12"))
    function_group_f9_f12(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "F5-F8"))
    function_group_f5_f8(addr);
}

// Loco function group - F0-F4
void function_group_f0_f4(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, 5uz> d{};
  for (auto i{5uz}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("F4-F0");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr,
       .bytes = dcc::make_f0_f4_packet(
         addr,
         static_cast<uint8_t>(d[4uz] << 4u | d[3uz] << 3u | d[2uz] << 2u |
                              d[1uz] << 1u | d[0uz] << 0u))});
}

// Loco function group - F9-F12
void function_group_f9_f12(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, 4uz> d{};
  for (auto i{4uz}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("F12-F9");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr,
       .bytes = dcc::make_f9_f12_packet(
         addr,
         static_cast<uint8_t>(d[3uz] << 3u | d[2uz] << 2u | d[1uz] << 1u |
                              d[0uz] << 0u))});
}

// Loco function group - F5-F8
void function_group_f5_f8(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, 4uz> d{};
  for (auto i{4uz}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("F8-F5");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr,
       .bytes = dcc::make_f5_f8_packet(
         addr,
         static_cast<uint8_t>(d[3uz] << 3u | d[2uz] << 2u | d[1uz] << 1u |
                              d[0uz] << 0u))});
}

// Loco feature expansion
void feature_expansion(dcc::Address addr) {
  ImGui::SeparatorText("Sub Instruction");
  static constexpr std::array instrs{"",
                                     "Binary State Control Long Form",
                                     "F29-F36",
                                     "F37-F44",
                                     "F45-F52",
                                     "F53-F60",
                                     "F61-F68",
                                     "Binary State Control Short Form",
                                     "F13-F20",
                                     "F21-F28"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
  if (!strcmp(instrs[static_cast<size_t>(i)], "Binary State Control Long Form"))
    feature_expansion_binary_state_control_long_form(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "F29-F36"))
    feature_expansion_f29_f36(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "F37-F44"))
    feature_expansion_f37_f44(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "F45-F52"))
    feature_expansion_f45_f52(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "F53-F60"))
    feature_expansion_f53_f60(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "F61-F68"))
    feature_expansion_f61_f68(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)],
                   "Binary State Control Short Form"))
    feature_expansion_binary_state_control_short_form(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "F13-F20"))
    feature_expansion_f13_f20(addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "F21-F28"))
    feature_expansion_f21_f28(addr);
}

// Loco feature expansion - binary state control long form
void feature_expansion_binary_state_control_long_form(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static bool d{};
  ImGui::Checkbox("State", &d);
  static uint16_t bin_addr{};
  ImGui::InputScalar("Address", ImGuiDataType_U16, &bin_addr);
  bin_addr = std::clamp<uint16_t>(bin_addr, 0u, 32767u);
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr,
       .bytes = dcc::make_binary_state_long_packet(addr, bin_addr, d)});
}

// Loco feature expansion - F29-F36
void feature_expansion_f29_f36(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, CHAR_BIT> d{};
  for (auto i{8uz}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("F36-F29");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr,
       .bytes = dcc::make_f29_f36_packet(
         addr,
         static_cast<uint8_t>(d[7uz] << 7u | d[6uz] << 6u | d[5uz] << 5u |
                              d[4uz] << 4u | d[3uz] << 3u | d[2uz] << 2u |
                              d[1uz] << 1u | d[0uz] << 0u))});
}

// Loco feature expansion - F37-F44
void feature_expansion_f37_f44(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, CHAR_BIT> d{};
  for (auto i{8uz}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("F44-F37");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr,
       .bytes = dcc::make_f37_f44_packet(
         addr,
         static_cast<uint8_t>(d[7uz] << 7u | d[6uz] << 6u | d[5uz] << 5u |
                              d[4uz] << 4u | d[3uz] << 3u | d[2uz] << 2u |
                              d[1uz] << 1u | d[0uz] << 0u))});
}

// Loco feature expansion - F45-F52
void feature_expansion_f45_f52(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, CHAR_BIT> d{};
  for (auto i{8uz}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("F52-F45");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr,
       .bytes = dcc::make_f45_f52_packet(
         addr,
         static_cast<uint8_t>(d[7uz] << 7u | d[6uz] << 6u | d[5uz] << 5u |
                              d[4uz] << 4u | d[3uz] << 3u | d[2uz] << 2u |
                              d[1uz] << 1u | d[0uz] << 0u))});
}

// Loco feature expansion - F53-F60
void feature_expansion_f53_f60(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, CHAR_BIT> d{};
  for (auto i{8uz}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("F60-F53");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr,
       .bytes = dcc::make_f53_f60_packet(
         addr,
         static_cast<uint8_t>(d[7uz] << 7u | d[6uz] << 6u | d[5uz] << 5u |
                              d[4uz] << 4u | d[3uz] << 3u | d[2uz] << 2u |
                              d[1uz] << 1u | d[0uz] << 0u))});
}

// Loco feature expansion - F61-F68
void feature_expansion_f61_f68(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, CHAR_BIT> d{};
  for (auto i{8uz}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("F68-F61");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr,
       .bytes = dcc::make_f61_f68_packet(
         addr,
         static_cast<uint8_t>(d[7uz] << 7u | d[6uz] << 6u | d[5uz] << 5u |
                              d[4uz] << 4u | d[3uz] << 3u | d[2uz] << 2u |
                              d[1uz] << 1u | d[0uz] << 0u))});
}

// Loco feature expansion - binary state control short form
void feature_expansion_binary_state_control_short_form(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static bool d{};
  ImGui::Checkbox("State", &d);
  static uint8_t bin_addr{0u};
  ImGui::InputScalar("Address", ImGuiDataType_U8, &bin_addr);
  bin_addr = std::clamp<uint8_t>(bin_addr, 0u, 127u);
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr,
       .bytes = dcc::make_binary_state_short_packet(addr, bin_addr, d)});
}

// Loco feature expansion - F13-F20
void feature_expansion_f13_f20(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, CHAR_BIT> d{};
  for (auto i{8uz}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("F20-F13");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr,
       .bytes = dcc::make_f13_f20_packet(
         addr,
         static_cast<uint8_t>(d[7uz] << 7u | d[6uz] << 6u | d[5uz] << 5u |
                              d[4uz] << 4u | d[3uz] << 3u | d[2uz] << 2u |
                              d[1uz] << 1u | d[0uz] << 0u))});
}

// Loco feature expansion - F21-F28
void feature_expansion_f21_f28(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, CHAR_BIT> d{};
  for (auto i{8uz}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("F28-F21");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr,
       .bytes = dcc::make_f21_f28_packet(
         addr,
         static_cast<uint8_t>(d[7uz] << 7u | d[6uz] << 6u | d[5uz] << 5u |
                              d[4uz] << 4u | d[3uz] << 3u | d[2uz] << 2u |
                              d[1uz] << 1u | d[0uz] << 0u))});
}

// Loco CV access
void cv_access(dcc::Address addr) {
  ImGui::SeparatorText("Sub Instruction");
  if (addr) {
    static constexpr std::array instrs{"", "Long Form", "Short Form", "XPOM"};
    static int i{};
    ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
    if (!strcmp(instrs[static_cast<size_t>(i)], "Long Form"))
      cv_access_long_form(addr);
    else if (!strcmp(instrs[static_cast<size_t>(i)], "Short Form"))
      cv_access_short_form(addr);
    else if (!strcmp(instrs[static_cast<size_t>(i)], "XPOM"))
      cv_access_xpom(addr);
  } else {
    static constexpr std::array instrs{"", "Long Form"};
    static int i{};
    ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
    if (!strcmp(instrs[static_cast<size_t>(i)], "Long Form"))
      cv_access_long_form(addr);
  }
}

// Loco CV access - long form
void cv_access_long_form(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static constexpr std::array instrs{
    "", "Byte Verify", "Byte Write", "Bit Verify", "Bit Write"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
  static uint16_t cv_addr{};
  ImGui::InputScalar("CV Address", ImGuiDataType_U16, &cv_addr);
  cv_addr = std::clamp<uint16_t>(cv_addr, 0u, 1023u);
  if (!strcmp(instrs[static_cast<size_t>(i)], "Byte Verify") ||
      !strcmp(instrs[static_cast<size_t>(i)], "Byte Write")) {
    static uint8_t cv_value{};
    ImGui::InputScalar("CV Value", ImGuiDataType_U8, &cv_value);
    ImGui::SeparatorText("Done");
    if (ImGui::Button("Push to Packets")) {
      if (!strcmp(instrs[static_cast<size_t>(i)], "Byte Verify"))
        state.packets.push_back(
          {.addr = addr,
           .bytes = addr ? dcc::make_cv_access_long_verify_packet(
                             addr, cv_addr, cv_value)
                         : dcc::make_cv_access_long_verify_service_packet(
                             cv_addr, cv_value)});
      else
        state.packets.push_back(
          {.addr = addr,
           .bytes = addr ? dcc::make_cv_access_long_write_packet(
                             addr, cv_addr, cv_value)
                         : dcc::make_cv_access_long_write_service_packet(
                             cv_addr, cv_value)});
    }
  } else if (!strcmp(instrs[static_cast<size_t>(i)], "Bit Verify") ||
             !strcmp(instrs[static_cast<size_t>(i)], "Bit Write")) {
    static bool bit{};
    ImGui::Checkbox("Bit", &bit);
    static constexpr uint8_t const min{};
    static constexpr uint8_t const max{7u};
    static uint8_t pos{};
    ImGui::SliderScalar("Position", ImGuiDataType_U8, &pos, &max, &min);
    ImGui::SeparatorText("Done");
    if (ImGui::Button("Push to Packets")) {
      if (!strcmp(instrs[static_cast<size_t>(i)], "Bit Verify"))
        state.packets.push_back(
          {.addr = addr,
           .bytes = addr ? dcc::make_cv_access_long_verify_packet(
                             addr, cv_addr, bit, pos)
                         : dcc::make_cv_access_long_verify_service_packet(
                             cv_addr, bit, pos)});
      else
        state.packets.push_back(
          {.addr = addr,
           .bytes = addr ? dcc::make_cv_access_long_write_packet(
                             addr, cv_addr, bit, pos)
                         : dcc::make_cv_access_long_write_service_packet(
                             cv_addr, bit, pos)});
    }
  }
}

// Loco CV access - short form
void cv_access_short_form(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static constexpr std::array instrs{"",
                                     "Acceleration Adjustment Value (CV23)",
                                     "Deceleration Adjustment Value (CV24)",
                                     "Long Address (CV17, 18 and 29)",
                                     "Indexed CVs (CV31 and 32)",
                                     "Long Consist Address (CV19 and 20)"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
  static std::array<uint8_t, 2uz> cvs{};
  uint8_t kkkk{};
  if (!strcmp(instrs[static_cast<size_t>(i)],
              "Acceleration Adjustment Value (CV23)")) {
    kkkk = 0b0010u;
    ImGui::InputScalar("CV23", ImGuiDataType_U8, &cvs[0uz]);
  } else if (!strcmp(instrs[static_cast<size_t>(i)],
                     "Deceleration Adjustment Value (CV24)")) {
    kkkk = 0b0011u;
    ImGui::InputScalar("CV24", ImGuiDataType_U8, &cvs[0uz]);
  } else if (!strcmp(instrs[static_cast<size_t>(i)],
                     "Long Address (CV17, 18 and 29)")) {
    kkkk = 0b0100u;
    ImGui::InputScalar("CV17", ImGuiDataType_U8, &cvs[0uz]);
    ImGui::InputScalar("CV18", ImGuiDataType_U8, &cvs[1uz]);
  } else if (!strcmp(instrs[static_cast<size_t>(i)],
                     "Indexed CVs (CV31 and 32)")) {
    kkkk = 0b0101u;
    ImGui::InputScalar("CV31", ImGuiDataType_U8, &cvs[0uz]);
    ImGui::InputScalar("CV32", ImGuiDataType_U8, &cvs[1uz]);
  } else if (!strcmp(instrs[static_cast<size_t>(i)],
                     "Long Consist Address (CV19 and 20)")) {
    kkkk = 0b0110u;
    ImGui::InputScalar("CV19", ImGuiDataType_U8, &cvs[0uz]);
    ImGui::InputScalar("CV20", ImGuiDataType_U8, &cvs[1uz]);
  }
  if (!kkkk) return;
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back({.addr = addr,
                             .bytes = dcc::make_cv_access_short_write_packet(
                               addr, kkkk, cvs[0uz], cvs[1uz])});
}

// Loco CV access - XPOM
void cv_access_xpom(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static constexpr std::array instrs{
    "", "Bytes Verify", "Bytes Write", "Bit Write"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
  static constexpr std::array sss{
    "00 (ID8)", "01 (ID9)", "10 (ID10)", "11 (ID11)"};
  static int ss{};
  ImGui::Combo("Sequence #", &ss, data(sss), ssize(sss));
  static uint32_t cv_addr{};
  ImGui::InputScalar("CV Address", ImGuiDataType_U32, &cv_addr);
  cv_addr = std::clamp<uint32_t>(cv_addr, 0u, 16777215u);
  if (!strcmp(instrs[static_cast<size_t>(i)], "Bytes Verify")) {
    ImGui::SeparatorText("Done");
    if (ImGui::Button("Push to Packets"))
      state.packets.push_back({.addr = addr,
                               .bytes = dcc::make_cv_access_xpom_verify_packet(
                                 addr, static_cast<uint8_t>(ss), cv_addr)});
  } else if (!strcmp(instrs[static_cast<size_t>(i)], "Bytes Write")) {
    static ztl::inplace_vector<uint8_t, 4uz> cv_values{0u};
    if (ImGui::Button("-") && size(cv_values) > 1uz) cv_values.pop_back();
    ImGui::SameLine();
    if (ImGui::Button("+") && size(cv_values) < cv_values.max_size())
      cv_values.push_back(0u);
    ImGui::SameLine();
    ImGui::Text("Length");
    ImGui::InputScalarN(
      "CV Values", ImGuiDataType_U8, data(cv_values), size(cv_values));
    ImGui::SeparatorText("Done");
    if (ImGui::Button("Push to Packets")) switch (size(cv_values)) {
        case 1uz:
          state.packets.push_back(
            {.addr = addr,
             .bytes = dcc::make_cv_access_xpom_write_packet(
               addr, static_cast<uint8_t>(ss), cv_addr, cv_values[0uz])});
          break;
        case 2uz:
          state.packets.push_back(
            {.addr = addr,
             .bytes =
               dcc::make_cv_access_xpom_write_packet(addr,
                                                     static_cast<uint8_t>(ss),
                                                     cv_addr,
                                                     cv_values[0uz],
                                                     cv_values[1uz])});
          break;
        case 3uz:
          state.packets.push_back(
            {.addr = addr,
             .bytes =
               dcc::make_cv_access_xpom_write_packet(addr,
                                                     static_cast<uint8_t>(ss),
                                                     cv_addr,
                                                     cv_values[0uz],
                                                     cv_values[1uz],
                                                     cv_values[2uz])});
          break;
        case 4uz:
          state.packets.push_back(
            {.addr = addr,
             .bytes =
               dcc::make_cv_access_xpom_write_packet(addr,
                                                     static_cast<uint8_t>(ss),
                                                     cv_addr,
                                                     cv_values[0uz],
                                                     cv_values[1uz],
                                                     cv_values[2uz],
                                                     cv_values[3uz])});
          break;
      }
  } else if (!strcmp(instrs[static_cast<size_t>(i)], "Bit Write")) {
    static bool bit{};
    ImGui::Checkbox("Bit", &bit);
    static constexpr uint8_t const min{};
    static constexpr uint8_t const max{7u};
    static uint8_t pos{};
    ImGui::SliderScalar("Position", ImGuiDataType_U8, &pos, &max, &min);
    ImGui::SeparatorText("Done");
    if (ImGui::Button("Push to Packets"))
      state.packets.push_back(
        {.addr = addr,
         .bytes = dcc::make_cv_access_xpom_write_packet(
           addr, static_cast<uint8_t>(ss), cv_addr, bit, pos)});
  }
}

} // namespace loco

////////////////////////////////////////////////////////////////////////////////

namespace accessory {

// Basic accessory
void basic() {
  ImGui::SeparatorText("Address");
  static dcc::Address::value_type addr{12};
  ImGui::InputScalar(UNIQUE_LABEL(), ImGuiDataType_U16, &addr);
  addr = std::clamp<dcc::Address::value_type>(addr, 0u, 2047u);
  accessory({.value = addr, .type = dcc::Address::BasicAccessory});
}

// Extended accessory
void extended() {
  ImGui::SeparatorText("Address");
  static dcc::Address::value_type addr{12};
  ImGui::InputScalar(UNIQUE_LABEL(), ImGuiDataType_U16, &addr);
  addr = std::clamp<dcc::Address::value_type>(addr, 0u, 2047u);
  accessory({.value = addr, .type = dcc::Address::ExtendedAccessory});
}

// Accessory
void accessory(dcc::Address addr) {
  ImGui::SeparatorText("Instruction");
  if (addr.type == dcc::Address::BasicAccessory) {
    static constexpr std::array instrs{
      "", "Basic Accessory Decoder Control", "NOP"};
    static int i{};
    ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
    if (!strcmp(instrs[static_cast<size_t>(i)],
                "Basic Accessory Decoder Control"))
      basic_accessory_decoder_control(addr);
    else if (!strcmp(instrs[static_cast<size_t>(i)], "NOP"))
      nop_for_basic_and_extended_accessory(addr);
  } else if (addr.type == dcc::Address::ExtendedAccessory) {
    static constexpr std::array instrs{
      "", "Extended Accessory Decoder Control", "NOP"};
    static int i{};
    ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
    if (!strcmp(instrs[static_cast<size_t>(i)],
                "Extended Accessory Decoder Control"))
      extended_accessory_decoder_control(addr);
    else if (!strcmp(instrs[static_cast<size_t>(i)], "NOP"))
      nop_for_basic_and_extended_accessory(addr);
  }
}

// Basic accessory decoder control
void basic_accessory_decoder_control(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static bool r{};
  ImGui::Checkbox("Output", &r);
  static bool d{};
  ImGui::Checkbox("State", &d);
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr, .bytes = dcc::make_basic_accessory_packet(addr, r, d)});
}

// Extended accessory decoder control
void extended_accessory_decoder_control(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static constexpr std::array types{"", "Aspect", "Switching Time"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(types), ssize(types));
  if (!strcmp(types[static_cast<size_t>(i)], "Aspect")) {
    static uint8_t dddddddd{};
    ImGui::InputScalar(UNIQUE_LABEL(), ImGuiDataType_U8, &dddddddd);
    ImGui::SeparatorText("Done");
    if (ImGui::Button("Push to Packets"))
      state.packets.push_back(
        {.addr = addr,
         .bytes = dcc::make_extended_accessory_packet(addr, dddddddd)});
  } else if (!strcmp(types[static_cast<size_t>(i)], "Switching Time")) {
    static bool r{};
    ImGui::Checkbox("Output", &r);
    static constexpr uint8_t min{0u};
    static constexpr uint8_t max{127u};
    static uint8_t zzzzzzz{};
    ImGui::SliderScalar("[s]",
                        ImGuiDataType_U8,
                        &zzzzzzz,
                        &min,
                        &max,
                        switch_on_time_labels[zzzzzzz]);
    ImGui::SeparatorText("Done");
    if (ImGui::Button("Push to Packets"))
      state.packets.push_back(
        {.addr = addr,
         .bytes = dcc::make_extended_accessory_packet(addr, r, zzzzzzz)});
  }
}

// Basic or extended accessory NOP
void nop_for_basic_and_extended_accessory(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  ImGui::Text("None");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = addr, .bytes = dcc::make_accessory_nop_packet(addr)});
}

} // namespace accessory

////////////////////////////////////////////////////////////////////////////////

namespace automatic_logon {

void automatic_logon() { ImGui::TextUnformatted("\\todo"); }

} // namespace automatic_logon

////////////////////////////////////////////////////////////////////////////////

namespace idle {

// Idle
void idle() {
  ImGui::SeparatorText("Instruction");
  static constexpr std::array instrs{"", "Digital Decoder Idle"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
  if (!strcmp(instrs[static_cast<size_t>(i)], "Digital Decoder Idle"))
    digital_decoder_idle({.value = 0xFFu, .type = dcc::Address::Idle});
}

// Idle digital decoder idle
void digital_decoder_idle(dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  ImGui::Text("None");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back({.addr = addr, .bytes = dcc::make_idle_packet()});
}

} // namespace idle

////////////////////////////////////////////////////////////////////////////////

namespace service {

// Service
void service() {
  ImGui::SeparatorText("Instruction");
  static constexpr std::array instrs{"", "CV Access"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
  if (!strcmp(instrs[static_cast<size_t>(i)], "CV Access"))
    ::loco::cv_access({.type = dcc::Address::UnknownService});
}

} // namespace service

////////////////////////////////////////////////////////////////////////////////

namespace user_defined {

// User defined
void user_defined() {
  ImGui::SeparatorText("Data");
  static dcc::Packet packet{0u, 0u};
  if (ImGui::Button("-") && size(packet) > 2uz) packet.pop_back();
  ImGui::SameLine();
  if (ImGui::Button("+") && size(packet) < packet.max_size())
    packet.push_back(0u);
  ImGui::SameLine();
  ImGui::Text("Length");
  ImGui::Separator();
  ImGui::BinaryTable(
    UNIQUE_LABEL(), data(packet), static_cast<int>(ssize(packet)));
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.packets.push_back(
      {.addr = dcc::decode_address(packet), .bytes = packet});
}

} // namespace user_defined

} // namespace

// Packet builder window
void packet_builder() {
  if (!state.windows.show_packet_builder) return;
  if (ImGui::Begin("Packet Builder",
                   &state.windows.show_packet_builder,
                   ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::SeparatorText("Type");
    static constexpr std::array types{"",
                                      "Broadcast",
                                      "Basic Loco",
                                      "Basic Accessory",
                                      "Extended Accessory",
                                      "Extended Loco",
                                      // "Automatic Logon",
                                      "Idle",
                                      "Service",
                                      "User Defined"};
    static int i{};
    ImGui::Combo(UNIQUE_LABEL(), &i, data(types), ssize(types));
    if (!strcmp(types[static_cast<size_t>(i)], "Broadcast"))
      broadcast::broadcast();
    else if (!strcmp(types[static_cast<size_t>(i)], "Basic Loco"))
      loco::basic();
    else if (!strcmp(types[static_cast<size_t>(i)], "Basic Accessory"))
      accessory::basic();
    else if (!strcmp(types[static_cast<size_t>(i)], "Extended Accessory"))
      accessory::extended();
    else if (!strcmp(types[static_cast<size_t>(i)], "Extended Loco"))
      loco::extended();
    else if (!strcmp(types[static_cast<size_t>(i)], "Automatic Logon"))
      automatic_logon::automatic_logon();
    else if (!strcmp(types[static_cast<size_t>(i)], "Idle")) idle::idle();
    else if (!strcmp(types[static_cast<size_t>(i)], "Service"))
      service::service();
    else if (!strcmp(types[static_cast<size_t>(i)], "User Defined"))
      user_defined::user_defined();
  }
  ImGui::End();
}
