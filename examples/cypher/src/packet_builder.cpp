#include "packet_builder.hpp"
#include <imgui.h>
#include <implot.h>
#include "utility.hpp"

namespace {

namespace broadcast {

// clang-format off
void broadcast(State& state);
  void decoder_control(State& state);
    void decoder_control_digital_decoder_reset(State& state);
  void speed_and_direction(State& state);
  void feature_expansion(State& state);
    void feature_expansion_time_and_date(State& state);
    void feature_expansion_system_time(State& state);
    void feature_expansion_command_station_feature_identification(State& state);
// clang-format on

} // namespace broadcast

namespace loco {

// clang-format off
void basic(State& state);
void extended(State& state);
  void loco(State& state, dcc::Address addr);
    void decoder_control(State& state, dcc::Address addr);
      void decoder_control_digital_decoder_reset(State& state, dcc::Address addr);
      void decoder_control_hard_reset(State& state, dcc::Address addr);
      void decoder_control_factory_test(State& state, dcc::Address addr);
      void decoder_control_set_advanced_addressing(State& state, dcc::Address addr);
      void decoder_control_decoder_acknowledgement_request(State& state, dcc::Address addr);
    void consist_control(State& state, dcc::Address addr);
      void consist_control_set_consist_address(State& state, dcc::Address addr);
    void advanced_operations(State& state, dcc::Address addr);
      void advanced_operations_speed_direction_and_function(State& state, dcc::Address addr);
      void advanced_operations_analog_function_group(State& state, dcc::Address addr);
      void advanced_operations_special_operating_modes(State& state, dcc::Address addr);
      void advanced_operations_128_speed_step_control(State& state, dcc::Address addr);
    void speed_and_direction(State& state, dcc::Address addr);
    void function_group(State& state, dcc::Address addr);
      void function_group_f0_f4(State& state, dcc::Address addr);
      void function_group_f9_f12(State& state, dcc::Address addr);
      void function_group_f5_f8(State& state, dcc::Address addr);
    void feature_expansion(State& state, dcc::Address addr);
      void feature_expansion_binary_state_control_long_form(State& state, dcc::Address addr);
      void feature_expansion_f29_f36(State& state, dcc::Address addr);
      void feature_expansion_f37_f44(State& state, dcc::Address addr);
      void feature_expansion_f45_f52(State& state, dcc::Address addr);
      void feature_expansion_f53_f60(State& state, dcc::Address addr);
      void feature_expansion_f61_f68(State& state, dcc::Address addr);
      void feature_expansion_binary_state_control_short_form(State& state, dcc::Address addr);
      void feature_expansion_f13_f20(State& state, dcc::Address addr);
      void feature_expansion_f21_f28(State& state, dcc::Address addr);
    void cv_access(State& state, dcc::Address addr);
      void cv_access_long_form(State& state, dcc::Address addr);
      void cv_access_short_form(State& state, dcc::Address addr);
      void cv_access_xpom(State& state, dcc::Address addr);
// clang-format on

} // namespace loco

namespace accessory {

// clang-format off
void basic(State& state);
void extended(State& state);
  void accessory(State& state, dcc::Address addr);
    void basic_accessory_decoder_control(State& state, dcc::Address addr);
    void extended_accessory_decoder_control(State& state, dcc::Address addr);
    void nop_for_basic_and_extended_accessory(State& state, dcc::Address addr);
// clang-format on

} // namespace accessory

namespace idle {

// clang-format off
void idle(State& state);
  void digital_decoder_idle(State& state);
// clang-format on

} // namespace idle

namespace broadcast {

//
void broadcast(State& state) {
  ImGui::SeparatorText("Instruction");
  static constexpr std::array instrs{
    "", "Decoder Control", "Speed & Direction", "Feature Expansion"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
  if (!strcmp(instrs[static_cast<size_t>(i)], "Decoder Control"))
    decoder_control(state);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Speed & Direction"))
    speed_and_direction(state);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Feature Expansion"))
    feature_expansion(state);
}

//
void decoder_control(State& state) {
  ImGui::SeparatorText("Sub Instruction");
  static constexpr std::array instrs{"", "Digital Decoder Reset"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
  if (!strcmp(instrs[static_cast<size_t>(i)], "Digital Decoder Reset"))
    decoder_control_digital_decoder_reset(state);
}

//
void decoder_control_digital_decoder_reset(State& state) {
  ImGui::SeparatorText("Parameters");
  ImGui::Text("None");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back({.bytes = dcc::make_reset_packet()});
}

// Digital Decoder Broadcast Stop Packets For All Decoders [S 9.2]
void speed_and_direction(State& state) {
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
  static int8_t speed{0};
  speed = std::clamp(speed, min, max);
  ImGui::SliderScalar("Speed",
                      ImGuiDataType_S8,
                      &speed,
                      &min,
                      &max,
                      speed_labels[static_cast<size_t>(speed + 1)]);
  rggggg = dcc::encode_rggggg(r, speed) | c << 4u;
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_speed_and_direction_packet(
         {.type = dcc::Address::Broadcast}, rggggg)});
}

//
void feature_expansion(State& state) {
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
    ::loco::feature_expansion_binary_state_control_long_form(
      state, {.type = dcc::Address::Broadcast});
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Time and Date"))
    feature_expansion_time_and_date(state);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "System Time"))
    feature_expansion_system_time(state);
  else if (!strcmp(instrs[static_cast<size_t>(i)],
                   "Command Station Feature Identification"))
    feature_expansion_command_station_feature_identification(state);
  else if (!strcmp(instrs[static_cast<size_t>(i)],
                   "Binary State Control Short Form"))
    ::loco::feature_expansion_binary_state_control_short_form(
      state, {.type = dcc::Address::Broadcast});
}

//
void feature_expansion_time_and_date(State& state) {
  ImGui::SeparatorText("Parameters");
  static constexpr std::array chrono_type{"", "Time", "Date", "Time Scale"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(chrono_type), ssize(chrono_type));
  if (!strcmp(chrono_type[static_cast<size_t>(i)], "Time")) {
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
      state.operations_packets.push_back(
        {.bytes =
           dcc::make_time_packet(weekday, time[0uz], time[1uz], acc, update)});
  } else if (!strcmp(chrono_type[static_cast<size_t>(i)], "Date")) {
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
      state.operations_packets.push_back(
        {.bytes = dcc::make_date_packet(static_cast<uint8_t>(date[0uz]),
                                        static_cast<uint8_t>(date[1uz]),
                                        date[2uz])});
  } else if (!strcmp(chrono_type[static_cast<size_t>(i)], "Time Scale")) {
    static float scale{};
    ImGui::InputFloat("Time Scale", &scale);
    ImGui::SeparatorText("Done");
    if (ImGui::Button("Push to Packets"))
      state.operations_packets.push_back(
        {.bytes = dcc::make_time_scale_packet(static_cast<FLOAT16>(scale))});
  }
}

//
void feature_expansion_system_time(State& state) {
  ImGui::SeparatorText("Parameters");
  static uint16_t ms{};
  ImGui::InputScalar("[ms]", ImGuiDataType_U16, &ms);
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_system_time_packet(ms)});
}

//
void feature_expansion_command_station_feature_identification(State& state) {
  ImGui::SeparatorText("Parameters");
  static constexpr std::array feat_type{
    "", "Loco Features", "Accessory and Broadcast Features", "BiDi Features"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(feat_type), ssize(feat_type));
  if (!strcmp(feat_type[static_cast<size_t>(i)], "Loco Features")) {
    static bool special_operating_modes{};
    static bool analog_function{};
    static bool binary_state_long{};
    static bool binary_state_short{};
    static bool f29_f68{};
    static bool f13_f28{};
    static bool xpom_write{};
    static bool pom_write{};
    static bool sdf{};
    static bool speed_steps_128{};
    static bool extended_addresses_10000_10239{};
    static bool basic_addresses_100_127_as_extended{};
    ImGui::Checkbox("Special Operating Modes", &special_operating_modes);
    ImGui::Checkbox("Analog Function", &analog_function);
    ImGui::Checkbox("Binary State Long", &binary_state_long);
    ImGui::Checkbox("Binary State Short", &binary_state_short);
    ImGui::Checkbox("F29-F68", &f29_f68);
    ImGui::Checkbox("F13-F28", &f13_f28);
    ImGui::Checkbox("XPOM Write", &xpom_write);
    ImGui::Checkbox("POM Write", &pom_write);
    ImGui::Checkbox("Speed, Direction and Functions", &sdf);
    ImGui::Checkbox("128 Speed Steps", &speed_steps_128);
    ImGui::Checkbox("Extended Addresses 10000-10239",
                    &extended_addresses_10000_10239);
    ImGui::Checkbox("Basic Addresses 100-127 as Extended",
                    &basic_addresses_100_127_as_extended);
    ImGui::SeparatorText("Done");
    if (ImGui::Button("Push to Packets"))
      state.operations_packets.push_back(
        {.bytes = dcc::make_command_station_feature_identification_packet(
           dcc::LocoFeatures{static_cast<uint16_t>(
             special_operating_modes << 13u | analog_function << 12u |
             binary_state_long << 11u | binary_state_short << 10u |
             f29_f68 << 9u | f13_f28 << 8u | xpom_write << 5u |
             pom_write << 4u | sdf << 3u | speed_steps_128 << 2u |
             extended_addresses_10000_10239 << 1u |
             basic_addresses_100_127_as_extended << 0u)})});
  } else if (!strcmp(feat_type[static_cast<size_t>(i)],
                     "Accessory and Broadcast Features")) {
    static bool system_time{};
    static bool time_scale{};
    static bool date{};
    static bool time{};
    static bool pom_write{};
    static bool extended{};
    static bool addresses_offset_by_4{};
    ImGui::Checkbox("System Time", &system_time);
    ImGui::Checkbox("Time Scale", &time_scale);
    ImGui::Checkbox("Date", &date);
    ImGui::Checkbox("Time", &time);
    ImGui::Checkbox("POM Write", &pom_write);
    ImGui::Checkbox("Extended", &extended);
    ImGui::Checkbox("Addresses Offset by 4", &addresses_offset_by_4);
    ImGui::SeparatorText("Done");
    if (ImGui::Button("Push to Packets"))
      state.operations_packets.push_back(
        {.bytes = dcc::make_command_station_feature_identification_packet(
           dcc::AccessoryBroadcastFeatures{static_cast<uint16_t>(
             system_time << 11u | time_scale << 10u | date << 9u | time << 8u |
             pom_write << 3u | extended << 1u |
             addresses_offset_by_4 << 0u)})});
  } else if (!strcmp(feat_type[static_cast<size_t>(i)], "BiDi Features")) {
    static bool railcom_plus{};
    static bool app_dyn_track_voltage{};
    static bool app_dyn_operating_params{};
    static bool app_dyn_container_levels{};
    static bool xpom_read{};
    static bool pom_read{};
    static bool nop_for_accessories{};
    static bool dcc_a{};
    static bool railcom{};
    ImGui::Checkbox("RailCom+", &railcom_plus);
    ImGui::Checkbox("app:dyn Track Voltage", &app_dyn_track_voltage);
    ImGui::Checkbox("app:dyn Operating Parameters", &app_dyn_operating_params);
    ImGui::Checkbox("app:dyn Container Levels", &app_dyn_container_levels);
    ImGui::Checkbox("XPOM Read", &xpom_read);
    ImGui::Checkbox("POM Read", &pom_read);
    ImGui::Checkbox("NOP for Accessories", &nop_for_accessories);
    ImGui::Checkbox("DCC-A", &dcc_a);
    ImGui::Checkbox("RailCom", &railcom);
    ImGui::SeparatorText("Done");
    if (ImGui::Button("Push to Packets"))
      state.operations_packets.push_back(
        {.bytes = dcc::make_command_station_feature_identification_packet(
           dcc::BiDiFeatures{static_cast<uint16_t>(
             railcom_plus << 15u | app_dyn_track_voltage << 10u |
             app_dyn_operating_params << 9u | app_dyn_container_levels << 8u |
             xpom_read << 4u | pom_read << 3u | nop_for_accessories << 2u |
             dcc_a << 1u | railcom << 0u)})});
  }
}

} // namespace broadcast

////////////////////////////////////////////////////////////////////////////////

namespace loco {

//
void basic(State& state) {
  ImGui::SeparatorText("Address");
  static dcc::Address::value_type addr{3};
  ImGui::InputScalar(UNIQUE_LABEL(), ImGuiDataType_U16, &addr);
  addr = std::clamp<dcc::Address::value_type>(addr, 1u, 127u);
  loco(state, {.value = addr, .type = dcc::Address::BasicLoco});
}

//
void extended(State& state) {
  ImGui::SeparatorText("Address");
  static dcc::Address::value_type addr{3};
  ImGui::InputScalar(UNIQUE_LABEL(), ImGuiDataType_U16, &addr);
  addr = std::clamp<dcc::Address::value_type>(addr, 1u, 10239u);
  loco(state, {.value = addr, .type = dcc::Address::ExtendedLoco});
}

//
void loco(State& state, dcc::Address addr) {
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
    decoder_control(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Consist Control"))
    consist_control(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Advanced Operations"))
    advanced_operations(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Speed & Direction"))
    speed_and_direction(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Function Group"))
    function_group(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Feature Expansion"))
    feature_expansion(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "CV Access"))
    cv_access(state, addr);
}

//
void decoder_control(State& state, dcc::Address addr) {
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
    decoder_control_digital_decoder_reset(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Hard Reset"))
    decoder_control_hard_reset(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Factory Test"))
    decoder_control_factory_test(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Set Advanced Addressing"))
    decoder_control_set_advanced_addressing(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)],
                   "Decoder Acknowledgement Request"))
    decoder_control_decoder_acknowledgement_request(state, addr);
}

//
void decoder_control_digital_decoder_reset(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  ImGui::Text("None");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back({.bytes = dcc::make_reset_packet(addr)});
}

//
void decoder_control_hard_reset(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  ImGui::Text("None");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_hard_reset_packet(addr)});
}

//
void decoder_control_factory_test(State& state, dcc::Address addr) {
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
    state.operations_packets.push_back(
      {.bytes = dcc::make_factory_test_packet(addr, bit0, bytes)});
}

//
void decoder_control_set_advanced_addressing(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static bool cv29_5{};
  ImGui::Checkbox("CV29:5", &cv29_5);
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_set_advanced_addressing_packet(addr, cv29_5)});
}

//
void decoder_control_decoder_acknowledgement_request(State& state,
                                                     dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  ImGui::Text("None");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Decoder Acknowledgement Request"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_ack_request_packet(addr)});
}

//
void consist_control(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Sub Instruction");
  static constexpr std::array instrs{"", "Set Consist Address"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
  if (!strcmp(instrs[static_cast<size_t>(i)], "Set Consist Address"))
    consist_control_set_consist_address(state, addr);
}

//
void consist_control_set_consist_address(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static dcc::Address::value_type consist_addr{3};
  ImGui::InputScalar("Address", ImGuiDataType_U8, &consist_addr);
  consist_addr = std::clamp<dcc::Address::value_type>(consist_addr, 1u, 127u);
  static bool r{};
  ImGui::Checkbox("Reversed", &r);
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes =
         dcc::make_set_consist_address_packet(addr, r << 7u | consist_addr)});
}

//
void advanced_operations(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Sub Instruction");
  static constexpr std::array instrs{"",
                                     "Speed, Direction and Functions",
                                     "Analog Function Group",
                                     "Special Operating Modes",
                                     "128 Speed Step Control"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
  if (!strcmp(instrs[static_cast<size_t>(i)], "Speed, Direction and Functions"))
    advanced_operations_speed_direction_and_function(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Analog Function Group"))
    advanced_operations_analog_function_group(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "Special Operating Modes"))
    advanced_operations_special_operating_modes(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "128 Speed Step Control"))
    advanced_operations_128_speed_step_control(state, addr);
}

//
void advanced_operations_speed_direction_and_function(State& state,
                                                      dcc::Address addr) {
  ImGui::SeparatorText("Parameters");

  // Speed and direction
  static uint8_t rggggggg{ztl::mask<7u>};
  auto r{static_cast<bool>(rggggggg & ztl::mask<7u>)};
  ImGui::Checkbox("Direction", &r);
  static constexpr int8_t const min{-1};
  static constexpr int8_t const max{126};
  static int8_t speed{0};
  speed = std::clamp(speed, min, max);
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
        state.operations_packets.push_back(
          {.bytes = make_speed_direction_and_functions_packet(
             addr, rggggggg, fs[0uz])});
        break;
      case 16uz:
        state.operations_packets.push_back(
          {.bytes = make_speed_direction_and_functions_packet(
             addr, rggggggg, fs[0uz], fs[1uz])});
        break;
      case 24uz:
        state.operations_packets.push_back(
          {.bytes = make_speed_direction_and_functions_packet(
             addr, rggggggg, fs[0uz], fs[1uz], fs[2uz])});
        break;
      case 32uz:
        state.operations_packets.push_back(
          {.bytes = make_speed_direction_and_functions_packet(
             addr, rggggggg, fs[0uz], fs[1uz], fs[2uz], fs[3uz])});
        break;
    }
  }
}

//
void advanced_operations_analog_function_group(State& state,
                                               dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static int ssssssss{};
  ImGui::Combo("Channel", &ssssssss, data(analog_labels), ssize(analog_labels));
  static uint8_t dddddddd{};
  ImGui::InputScalar("Value", ImGuiDataType_U8, &dddddddd);
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_analog_function_group_packet(
         addr, static_cast<uint8_t>(ssssssss), dddddddd)});
}

///
void advanced_operations_special_operating_modes(State& state,
                                                 dcc::Address addr) {
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
    state.operations_packets.push_back(
      {.bytes = dcc::make_special_operating_modes(
         addr,
         static_cast<dcc::Consist>(((i & 1) << 1) | ((i & 2) >> 1) | (i & ~3)),
         shunting,
         west,
         east,
         man)});
}

//
void advanced_operations_128_speed_step_control(State& state,
                                                dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static uint8_t rggggggg{ztl::mask<7u>};
  auto r{static_cast<bool>(rggggggg & ztl::mask<7u>)};
  ImGui::Checkbox("Direction", &r);
  static constexpr int8_t const min{-1};
  static constexpr int8_t const max{126};
  static int8_t speed{};
  speed = std::clamp(speed, min, max);
  ImGui::SliderScalar("Speed",
                      ImGuiDataType_S8,
                      &speed,
                      &min,
                      &max,
                      speed_labels[static_cast<size_t>(speed + 1)]);
  rggggggg = dcc::encode_rggggggg(r, speed);
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_128_speed_step_control_packet(addr, rggggggg)});
}

//
void speed_and_direction(State& state, dcc::Address addr) {
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
  static int8_t speed{0};
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
    state.operations_packets.push_back(
      {.bytes = dcc::make_speed_and_direction_packet(addr, rggggg)});
}

//
void function_group(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Sub Instruction");
  static constexpr std::array instrs{"", "F0-F4", "F9-F12", "F5-F8"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
  if (!strcmp(instrs[static_cast<size_t>(i)], "F0-F4"))
    function_group_f0_f4(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "F9-F12"))
    function_group_f9_f12(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "F5-F8"))
    function_group_f5_f8(state, addr);
}

//
void function_group_f0_f4(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, 5uz> d{};
  for (auto i{5uz}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("F4-F0");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_f0_f4_packet(
         addr,
         static_cast<uint8_t>(d[4uz] << 4u | d[3uz] << 3u | d[2uz] << 2u |
                              d[1uz] << 1u | d[0uz] << 0u))});
}

//
void function_group_f9_f12(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, 4uz> d{};
  for (auto i{4uz}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("F12-F9");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_f9_f12_packet(
         addr,
         static_cast<uint8_t>(d[3uz] << 3u | d[2uz] << 2u | d[1uz] << 1u |
                              d[0uz] << 0u))});
}

//
void function_group_f5_f8(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, 4uz> d{};
  for (auto i{4uz}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("F8-F5");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_f5_f8_packet(
         addr,
         static_cast<uint8_t>(d[3uz] << 3u | d[2uz] << 2u | d[1uz] << 1u |
                              d[0uz] << 0u))});
}

//
void feature_expansion(State& state, dcc::Address addr) {
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
    feature_expansion_binary_state_control_long_form(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "F29-F36"))
    feature_expansion_f29_f36(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "F37-F44"))
    feature_expansion_f37_f44(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "F45-F52"))
    feature_expansion_f45_f52(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "F53-F60"))
    feature_expansion_f53_f60(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "F61-F68"))
    feature_expansion_f61_f68(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)],
                   "Binary State Control Short Form"))
    feature_expansion_binary_state_control_short_form(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "F13-F20"))
    feature_expansion_f13_f20(state, addr);
  else if (!strcmp(instrs[static_cast<size_t>(i)], "F21-F28"))
    feature_expansion_f21_f28(state, addr);
}

//
void feature_expansion_binary_state_control_long_form(State& state,
                                                      dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static bool d{};
  ImGui::Checkbox("State", &d);
  static uint16_t bin_addr{};
  ImGui::InputScalar("Address", ImGuiDataType_U16, &bin_addr);
  bin_addr = std::clamp<uint16_t>(bin_addr, 0u, 32767u);
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_binary_state_long_packet(addr, bin_addr, d)});
}

//
void feature_expansion_f29_f36(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, 8uz> d{};
  for (auto i{8uz}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("F36-F29");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_f29_f36_packet(
         addr,
         static_cast<uint8_t>(d[7uz] << 7u | d[6uz] << 6u | d[5uz] << 5u |
                              d[4uz] << 4u | d[3uz] << 3u | d[2uz] << 2u |
                              d[1uz] << 1u | d[0uz] << 0u))});
}

//
void feature_expansion_f37_f44(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, 8uz> d{};
  for (auto i{8uz}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("F44-F37");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_f37_f44_packet(
         addr,
         static_cast<uint8_t>(d[7uz] << 7u | d[6uz] << 6u | d[5uz] << 5u |
                              d[4uz] << 4u | d[3uz] << 3u | d[2uz] << 2u |
                              d[1uz] << 1u | d[0uz] << 0u))});
}

//
void feature_expansion_f45_f52(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, 8uz> d{};
  for (auto i{8uz}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("F52-F45");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_f45_f52_packet(
         addr,
         static_cast<uint8_t>(d[7uz] << 7u | d[6uz] << 6u | d[5uz] << 5u |
                              d[4uz] << 4u | d[3uz] << 3u | d[2uz] << 2u |
                              d[1uz] << 1u | d[0uz] << 0u))});
}

//
void feature_expansion_f53_f60(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, 8uz> d{};
  for (auto i{8uz}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("F60-F53");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_f53_f60_packet(
         addr,
         static_cast<uint8_t>(d[7uz] << 7u | d[6uz] << 6u | d[5uz] << 5u |
                              d[4uz] << 4u | d[3uz] << 3u | d[2uz] << 2u |
                              d[1uz] << 1u | d[0uz] << 0u))});
}

//
void feature_expansion_f61_f68(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, 8uz> d{};
  for (auto i{8uz}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("F68-F61");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_f61_f68_packet(
         addr,
         static_cast<uint8_t>(d[7uz] << 7u | d[6uz] << 6u | d[5uz] << 5u |
                              d[4uz] << 4u | d[3uz] << 3u | d[2uz] << 2u |
                              d[1uz] << 1u | d[0uz] << 0u))});
}

//
void feature_expansion_binary_state_control_short_form(State& state,
                                                       dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static bool d{};
  ImGui::Checkbox("State", &d);
  static uint8_t bin_addr{0u};
  ImGui::InputScalar("Address", ImGuiDataType_U8, &bin_addr);
  bin_addr = std::clamp<uint8_t>(bin_addr, 0u, 127u);
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_binary_state_short_packet(addr, bin_addr, d)});
}

//
void feature_expansion_f13_f20(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, 8uz> d{};
  for (auto i{8uz}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("F20-F13");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_f13_f20_packet(
         addr,
         static_cast<uint8_t>(d[7uz] << 7u | d[6uz] << 6u | d[5uz] << 5u |
                              d[4uz] << 4u | d[3uz] << 3u | d[2uz] << 2u |
                              d[1uz] << 1u | d[0uz] << 0u))});
}

//
void feature_expansion_f21_f28(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, 8uz> d{};
  for (auto i{8uz}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &d[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("F28-F21");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_f21_f28_packet(
         addr,
         static_cast<uint8_t>(d[7uz] << 7u | d[6uz] << 6u | d[5uz] << 5u |
                              d[4uz] << 4u | d[3uz] << 3u | d[2uz] << 2u |
                              d[1uz] << 1u | d[0uz] << 0u))});
}

//
void cv_access(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Sub Instruction");
  if (addr) {
    static constexpr std::array instrs{"", "Long Form", "Short Form", "XPOM"};
    static int i{};
    ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
    if (!strcmp(instrs[static_cast<size_t>(i)], "Long Form"))
      cv_access_long_form(state, addr);
    else if (!strcmp(instrs[static_cast<size_t>(i)], "Short Form"))
      cv_access_short_form(state, addr);
    else if (!strcmp(instrs[static_cast<size_t>(i)], "XPOM"))
      cv_access_xpom(state, addr);
  } else {
    static constexpr std::array instrs{"", "Long Form"};
    static int i{};
    ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
    if (!strcmp(instrs[static_cast<size_t>(i)], "Long Form"))
      cv_access_long_form(state, addr);
  }
}

//
void cv_access_long_form(State& state, dcc::Address addr) {
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
        addr ? state.operations_packets.push_back(
                 {.bytes = dcc::make_cv_access_long_verify_packet(
                    addr, cv_addr, cv_value)})
             : state.service_packets.push_back(
                 {.bytes = dcc::make_cv_access_long_verify_service_packet(
                    cv_addr, cv_value)});
      else
        addr ? state.operations_packets.push_back(
                 {.bytes = dcc::make_cv_access_long_write_packet(
                    addr, cv_addr, cv_value)})
             : state.service_packets.push_back(
                 {.bytes = dcc::make_cv_access_long_write_service_packet(
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
        addr ? state.operations_packets.push_back(
                 {.bytes = dcc::make_cv_access_long_verify_packet(
                    addr, cv_addr, bit, pos)})
             : state.service_packets.push_back(
                 {.bytes = dcc::make_cv_access_long_verify_service_packet(
                    cv_addr, bit, pos)});
      else
        addr ? state.operations_packets.push_back(
                 {.bytes = dcc::make_cv_access_long_write_packet(
                    addr, cv_addr, bit, pos)})
             : state.service_packets.push_back(
                 {.bytes = dcc::make_cv_access_long_write_service_packet(
                    cv_addr, bit, pos)});
    }
  }
}

//
void cv_access_short_form(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static constexpr std::array instr{"",
                                    "Acceleration Adjustment Value (CV23)",
                                    "Deceleration Adjustment Value (CV24)",
                                    "Long Address (CV17, 18 and 29)",
                                    "Indexed CVs (CV31 and 32)",
                                    "Long Consist Address (CV19 and 20)"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instr), ssize(instr));
  static std::array<uint8_t, 2uz> cvs{};
  uint8_t kkkk{};
  if (!strcmp(instr[static_cast<size_t>(i)],
              "Acceleration Adjustment Value (CV23)")) {
    kkkk = 0b0010u;
    ImGui::InputScalar("CV23", ImGuiDataType_U8, &cvs[0uz]);
  } else if (!strcmp(instr[static_cast<size_t>(i)],
                     "Deceleration Adjustment Value (CV24)")) {
    kkkk = 0b0011u;
    ImGui::InputScalar("CV24", ImGuiDataType_U8, &cvs[0uz]);
  } else if (!strcmp(instr[static_cast<size_t>(i)],
                     "Long Address (CV17, 18 and 29)")) {
    kkkk = 0b0100u;
    ImGui::InputScalar("CV17", ImGuiDataType_U8, &cvs[0uz]);
    ImGui::InputScalar("CV18", ImGuiDataType_U8, &cvs[1uz]);
  } else if (!strcmp(instr[static_cast<size_t>(i)],
                     "Indexed CVs (CV31 and 32)")) {
    kkkk = 0b0101u;
    ImGui::InputScalar("CV31", ImGuiDataType_U8, &cvs[0uz]);
    ImGui::InputScalar("CV32", ImGuiDataType_U8, &cvs[1uz]);
  } else if (!strcmp(instr[static_cast<size_t>(i)],
                     "Long Consist Address (CV19 and 20)")) {
    kkkk = 0b0110u;
    ImGui::InputScalar("CV19", ImGuiDataType_U8, &cvs[0uz]);
    ImGui::InputScalar("CV20", ImGuiDataType_U8, &cvs[1uz]);
  }
  if (!kkkk) return;
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_cv_access_short_write_packet(
         addr, kkkk, cvs[0uz], cvs[1uz])});
}

//
void cv_access_xpom(State& state, dcc::Address addr) {
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
      state.operations_packets.push_back(
        {.bytes = dcc::make_cv_access_xpom_verify_packet(
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
          state.operations_packets.push_back(
            {.bytes = dcc::make_cv_access_xpom_write_packet(
               addr, static_cast<uint8_t>(ss), cv_addr, cv_values[0uz])});
          break;
        case 2uz:
          state.operations_packets.push_back(
            {.bytes =
               dcc::make_cv_access_xpom_write_packet(addr,
                                                     static_cast<uint8_t>(ss),
                                                     cv_addr,
                                                     cv_values[0uz],
                                                     cv_values[1uz])});
          break;
        case 3uz:
          state.operations_packets.push_back(
            {.bytes =
               dcc::make_cv_access_xpom_write_packet(addr,
                                                     static_cast<uint8_t>(ss),
                                                     cv_addr,
                                                     cv_values[0uz],
                                                     cv_values[1uz],
                                                     cv_values[2uz])});
          break;
        case 4uz:
          state.operations_packets.push_back(
            {.bytes =
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
      state.operations_packets.push_back(
        {.bytes = dcc::make_cv_access_xpom_write_packet(
           addr, static_cast<uint8_t>(ss), cv_addr, bit, pos)});
  }
}

} // namespace loco

////////////////////////////////////////////////////////////////////////////////

namespace accessory {

//
void basic(State& state) {
  ImGui::SeparatorText("Address");
  static dcc::Address::value_type addr{12};
  ImGui::InputScalar(UNIQUE_LABEL(), ImGuiDataType_U16, &addr);
  addr = std::clamp<dcc::Address::value_type>(addr, 0u, 2047u);
  accessory(state, {.value = addr, .type = dcc::Address::BasicAccessory});
}

//
void extended(State& state) {
  ImGui::SeparatorText("Address");
  static dcc::Address::value_type addr{12};
  ImGui::InputScalar(UNIQUE_LABEL(), ImGuiDataType_U16, &addr);
  addr = std::clamp<dcc::Address::value_type>(addr, 0u, 2047u);
  accessory(state, {.value = addr, .type = dcc::Address::ExtendedAccessory});
}

//
void accessory(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Instruction");
  if (addr.type == dcc::Address::BasicAccessory) {
    static constexpr std::array instrs{
      "", "Basic Accessory Decoder Control", "NOP"};
    static int i{};
    ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
    if (!strcmp(instrs[static_cast<size_t>(i)],
                "Basic Accessory Decoder Control"))
      basic_accessory_decoder_control(state, addr);
    else if (!strcmp(instrs[static_cast<size_t>(i)], "NOP"))
      nop_for_basic_and_extended_accessory(state, addr);
  } else if (addr.type == dcc::Address::ExtendedAccessory) {
    static constexpr std::array instrs{
      "", "Extended Accessory Decoder Control", "NOP"};
    static int i{};
    ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
    if (!strcmp(instrs[static_cast<size_t>(i)],
                "Extended Accessory Decoder Control"))
      extended_accessory_decoder_control(state, addr);
    else if (!strcmp(instrs[static_cast<size_t>(i)], "NOP"))
      nop_for_basic_and_extended_accessory(state, addr);
  }
}

//
void basic_accessory_decoder_control(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static bool r{};
  ImGui::Checkbox("Output", &r);
  static bool d{};
  ImGui::Checkbox("State", &d);
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_basic_accessory_packet(addr, r, d)});
}

//
void extended_accessory_decoder_control(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  static constexpr std::array type{"", "Signal", "Switching Time"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(type), ssize(type));
  if (!strcmp(type[static_cast<size_t>(i)], "Signal")) {
    static uint8_t dddddddd{};
    ImGui::InputScalar("State", ImGuiDataType_U8, &dddddddd);
    ImGui::SeparatorText("Done");
    if (ImGui::Button("Push to Packets"))
      state.operations_packets.push_back(
        {.bytes = dcc::make_extended_accessory_packet(addr, dddddddd)});
  } else if (!strcmp(type[static_cast<size_t>(i)], "Switching Time")) {
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
      state.operations_packets.push_back(
        {.bytes = dcc::make_extended_accessory_packet(addr, r, zzzzzzz)});
  }
}

//
void nop_for_basic_and_extended_accessory(State& state, dcc::Address addr) {
  ImGui::SeparatorText("Parameters");
  ImGui::Text("None");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back(
      {.bytes = dcc::make_accessory_nop_packet(addr)});
}

} // namespace accessory

////////////////////////////////////////////////////////////////////////////////

namespace idle {

//
void idle(State& state) {
  ImGui::SeparatorText("Instruction");
  static constexpr std::array instrs{"", "Digital Decoder Idle"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
  if (!strcmp(instrs[static_cast<size_t>(i)], "Digital Decoder Idle"))
    digital_decoder_idle(state);
}

//
void digital_decoder_idle(State& state) {
  ImGui::SeparatorText("Parameters");
  ImGui::Text("None");
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Packets"))
    state.operations_packets.push_back({.bytes = dcc::make_idle_packet()});
}

} // namespace idle

////////////////////////////////////////////////////////////////////////////////

namespace service {

//
void service(State& state) {
  ImGui::SeparatorText("Instruction");
  static constexpr std::array instrs{"", "CV Access"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(instrs), ssize(instrs));
  if (!strcmp(instrs[static_cast<size_t>(i)], "CV Access"))
    ::loco::cv_access(state, {.type = dcc::Address::UnknownService});
}

} // namespace service

////////////////////////////////////////////////////////////////////////////////

namespace user_defined {

//
void user_defined(State& state) {
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
    state.operations_packets.push_back({.bytes = packet});
}

} // namespace user_defined

} // namespace

//
void packet_builder(State& state) {
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
      broadcast::broadcast(state);
    else if (!strcmp(types[static_cast<size_t>(i)], "Basic Loco"))
      loco::basic(state);
    else if (!strcmp(types[static_cast<size_t>(i)], "Basic Accessory"))
      accessory::basic(state);
    else if (!strcmp(types[static_cast<size_t>(i)], "Extended Accessory"))
      accessory::extended(state);
    else if (!strcmp(types[static_cast<size_t>(i)], "Extended Loco"))
      loco::extended(state);
    else if (!strcmp(types[static_cast<size_t>(i)], "Automatic Logon"))
      ; //
    else if (!strcmp(types[static_cast<size_t>(i)], "Idle")) idle::idle(state);
    else if (!strcmp(types[static_cast<size_t>(i)], "Service"))
      service::service(state);
    else if (!strcmp(types[static_cast<size_t>(i)], "User Defined"))
      user_defined::user_defined(state);
  }
  ImGui::End();
}
