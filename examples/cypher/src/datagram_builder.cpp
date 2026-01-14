#include "datagram_builder.hpp"
#include <imgui.h>
#include "state.hpp"
#include "utility.hpp"

namespace {

using namespace dcc::bidi;

namespace broadcast {

// clang-format off
void broadcast(State::Datagram& datagram);
  void channel2(State::Datagram& datagram);
    void app_search(State::Datagram& datagram);
// clang-format on

} // namespace broadcast

namespace loco {

// clang-format off
void loco(State::Datagram& datagram);
  void channel1(State::Datagram& datagram);
    void app_adr_high(State::Datagram& datagram);
    void app_adr_low(State::Datagram& datagram);
    void app_adr_info1(State::Datagram& datagram);
  void channel2(State::Datagram& datagram);
    void app_pom(State::Datagram& datagram);
    void app_ext(State::Datagram& datagram);
    void app_info(State::Datagram& datagram);
    template<size_t I>
    void app_dyn(State::Datagram& datagram, std::span<char const* const> labels);
    void app_xpom(State::Datagram& datagram);
    void app_cv_auto(State::Datagram& datagram);
    void app_block(State::Datagram& datagram);
// clang-format on

} // namespace loco

namespace broadcast {

// Broadcast
void broadcast(State::Datagram& datagram) {
  datagram.addr = {.type = dcc::Address::Broadcast}; // Default
  channel2(datagram);
  if (std::ranges::all_of(datagram.bytes, [](auto b) { return !b; })) return;
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Datagrams")) {
    state.datagrams.push_back(datagram);
  }
}

// Broadcast channel2
void channel2(State::Datagram& datagram) {
  ImGui::SeparatorText("Channel 2");
  static constexpr std::array apps{"", "app:search"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(apps), ssize(apps));
  if (!strcmp(apps[static_cast<size_t>(i)], "app:search")) app_search(datagram);
}

// Broadcast app:search
void app_search(State::Datagram& datagram) {
  ImGui::SeparatorText("Parameters");
  static dcc::Address::value_type addr{3};
  ImGui::InputScalar("Address", ImGuiDataType_U16, &addr);
  addr = std::clamp<dcc::Address::value_type>(addr, 1u, 10239u);
  static bool consist{};
  ImGui::Checkbox("Consist", &consist);
  static bool r{};
  if (consist) ImGui::Checkbox("Reversed", &r);
  static constexpr uint8_t min{0u};
  static constexpr uint8_t max{30u};
  static uint8_t s{};
  ImGui::SliderScalar("[s]", ImGuiDataType_U8, &s, &min, &max);
  std::ranges::copy(
    make_app_search_datagram(addr, static_cast<uint8_t>(r << 7u | consist), s),
    begin(datagram.bytes) + channel1_size);
}

} // namespace broadcast

////////////////////////////////////////////////////////////////////////////////

namespace loco {

// Loco
void loco(State::Datagram& datagram) {
  datagram.addr = {.value = 3u, .type = dcc::Address::BasicLoco}; // Default
  channel1(datagram);
  channel2(datagram);
  if (std::ranges::all_of(datagram.bytes, [](auto b) { return !b; })) return;
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Datagrams")) state.datagrams.push_back(datagram);
}

// Loco channel1
void channel1(State::Datagram& datagram) {
  ImGui::SeparatorText("Channel 1");
  static constexpr std::array apps{
    "", "app:adr_high", "app:adr_low", "app:info1"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(apps), ssize(apps));
  if (!strcmp(apps[static_cast<size_t>(i)], "app:adr_high"))
    app_adr_high(datagram);
  else if (!strcmp(apps[static_cast<size_t>(i)], "app:adr_low"))
    app_adr_low(datagram);
  else if (!strcmp(apps[static_cast<size_t>(i)], "app:info1"))
    app_adr_info1(datagram);
}

// Loco app:adr_high
void app_adr_high(State::Datagram& datagram) {
  ImGui::SeparatorText("Parameters");
  static dcc::Address::value_type addr{3};
  ImGui::InputScalar("Address", ImGuiDataType_U16, &addr);
  addr = std::clamp<dcc::Address::value_type>(addr, 1u, 10239u);
  static bool consist{};
  ImGui::Checkbox("Consist", &consist);
  std::ranges::copy(make_app_adr_high_datagram(addr, consist),
                    begin(datagram.bytes));
}

// Loco app:adr_low
void app_adr_low(State::Datagram& datagram) {
  ImGui::SeparatorText("Parameters");
  static dcc::Address::value_type addr{3};
  ImGui::InputScalar("Address", ImGuiDataType_U16, &addr);
  addr = std::clamp<dcc::Address::value_type>(addr, 1u, 10239u);
  static bool consist{};
  ImGui::Checkbox("Consist", &consist);
  static bool r{};
  if (consist) ImGui::Checkbox("Reversed", &r);
  std::ranges::copy(make_app_adr_low_datagram(addr, r << 7u | consist),
                    begin(datagram.bytes));
}

// Loco app:adr_info1
void app_adr_info1(State::Datagram& datagram) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, CHAR_BIT> f{};
  ImGui::Checkbox("Track Polarity", &f[0uz]);
  ImGui::Checkbox("East-West", &f[1uz]);
  ImGui::Checkbox("Driving", &f[2uz]);
  ImGui::Checkbox("Consist", &f[3uz]);
  ImGui::Checkbox("Addressing Request", &f[4uz]);
  auto const dg{make_app_info1_datagram(
    {static_cast<app::Info1::Flags>(f[4uz] << 4u | f[3uz] << 3u | f[2uz] << 2u |
                                    f[1uz] << 1u | f[0uz] << 0u)})};
  std::ranges::copy(dg, begin(datagram.bytes));
}

// Loco channel2
void channel2(State::Datagram& datagram) {
  ImGui::SeparatorText("Channel 2");
  static constexpr std::array apps{
    "",
    "app:pom",
    "app:ext",
    // "app:info",
    "app:dyn",
    "app:xpom",
    "app:CV-auto",
    // "app:block"
  };
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(apps), ssize(apps));
  if (!strcmp(apps[static_cast<size_t>(i)], "app:pom")) app_pom(datagram);
  else if (!strcmp(apps[static_cast<size_t>(i)], "app:ext")) app_ext(datagram);
  else if (!strcmp(apps[static_cast<size_t>(i)], "app:info"))
    app_info(datagram);
  else if (!strcmp(apps[static_cast<size_t>(i)], "app:dyn"))
    app_dyn<0uz>(datagram, mob_app_dyn_labels);
  else if (!strcmp(apps[static_cast<size_t>(i)], "app:xpom"))
    app_xpom(datagram);
  else if (!strcmp(apps[static_cast<size_t>(i)], "app:CV-auto"))
    app_cv_auto(datagram);
  else if (!strcmp(apps[static_cast<size_t>(i)], "app:block"))
    app_block(datagram);
}

// Loco app:pom
void app_pom(State::Datagram& datagram) {
  ImGui::SeparatorText("Parameters");
  static uint8_t cv_value{};
  ImGui::InputScalar("CV value", ImGuiDataType_U8, &cv_value);
  std::ranges::copy(make_app_pom_datagram(cv_value), begin(datagram.bytes) + 2);
}

// Loco app:ext
void app_ext(State::Datagram& datagram) {
  ImGui::SeparatorText("Parameters");
  static int i{};
  ImGui::Combo(
    UNIQUE_LABEL(), &i, data(app_ext_type_labels), ssize(app_ext_type_labels));
  static uint16_t p{0};
  ImGui::InputScalar("Position", ImGuiDataType_U16, &p);
  p = std::clamp<uint16_t>(
    p,
    0u,
    static_cast<uint16_t>(smath::pow(2u, i < app::Ext::Reserved8 ? 11u : 8u) -
                          1u));
  std::ranges::copy(
    make_app_ext_datagram(
      {.t = static_cast<app::Ext::Type>(
         i < app::Ext::Reserved8 ? 0u : static_cast<uint8_t>(i)),
       .p = p}),
    begin(datagram.bytes) + 2);
}

// Loco app:info
void app_info(State::Datagram&) {
  ImGui::SeparatorText("Parameters");
  ImGui::TextUnformatted("\\todo");
}

// Loco app:dyn
template<size_t I>
void app_dyn(State::Datagram& datagram, std::span<char const* const> labels) {
  ImGui::SeparatorText("Parameters");
  static int i{};
  ImGui::Combo(
    UNIQUE_LABEL(I), &i, data(labels), static_cast<int>(ssize(labels)));
  static uint8_t d{};
  // Speed 1
  if (!strcmp(labels[static_cast<size_t>(i)], "Speed 1")) {
    static constexpr uint16_t min{0u};
    static constexpr uint16_t max{255u};
    static uint16_t speed{};
    ImGui::SliderScalar(("[km/h]##" + std::to_string(I)).c_str(),
                        ImGuiDataType_U16,
                        &speed,
                        &min,
                        &max);
    d = static_cast<uint8_t>(speed);
  }
  // Speed 2
  else if (!strcmp(labels[static_cast<size_t>(i)], "Speed 2")) {
    static constexpr uint16_t min{256u};
    static constexpr uint16_t max{511u};
    static uint16_t speed{min};
    ImGui::SliderScalar(("[km/h]##" + std::to_string(I)).c_str(),
                        ImGuiDataType_U16,
                        &speed,
                        &min,
                        &max);
    d = static_cast<uint8_t>(speed - 256u);
  }
  // Load or speed steps
  else if (!strcmp(labels[static_cast<size_t>(i)], "Load or Speed Steps")) {
    static bool bit7{};
    ImGui::Checkbox(("Bit7##" + std::to_string(I)).c_str(), &bit7);
    if (bit7) {
      static constexpr int8_t const min{-1};
      static constexpr int8_t const max{126};
      static int8_t speed{0};
      ImGui::SliderScalar(("Speed Steps" + std::to_string(I)).c_str(),
                          ImGuiDataType_S8,
                          &speed,
                          &min,
                          &max,
                          speed_labels[static_cast<size_t>(speed + 1)]);
      d = dcc::encode_rggggggg(true, speed);
    } else {
      static constexpr uint8_t const min{0u};
      static constexpr uint8_t const max{127u};
      ImGui::SliderScalar(
        ("Load" + std::to_string(I)).c_str(), ImGuiDataType_U8, &d, &min, &max);
    }
  }
  // RailCom version
  else if (!strcmp(labels[static_cast<size_t>(i)], "RailCom Version")) {
    static std::array<uint8_t, 2uz> ver{};
    ImGui::InputScalarN(
      UNIQUE_LABEL(I), ImGuiDataType_U8, data(ver), ssize(ver));
    ver[0uz] = std::clamp<uint8_t>(ver[0uz], 0u, 9u);
    ver[1uz] = std::clamp<uint8_t>(ver[1uz], 0u, 9u);
    d = static_cast<uint8_t>(ver[0uz] << 4u | ver[1uz]);
  }
  // Change flags
  else if (!strcmp(labels[static_cast<size_t>(i)], "Change Flags")) {
    static std::array<bool, CHAR_BIT> f{};
    for (auto j{size(f)}; j-- > 0uz;) {
      ImGui::Checkbox((UNIQUE_LABEL(I) + std::to_string(j)).c_str(), &f[j]);
      ImGui::SameLine();
    }
    ImGui::TextUnformatted("?");
    d = static_cast<uint8_t>(f[7uz] << 7u | f[6uz] << 6u | f[5uz] << 5u |
                             f[4uz] << 4u | f[3uz] << 3u | f[2uz] << 2u |
                             f[1uz] << 1u | f[0uz] << 0u);
  }
  // Flag register
  else if (!strcmp(labels[static_cast<size_t>(i)], "Flag Register")) {
    static std::array<bool, CHAR_BIT> f{};
    for (auto j{size(f)}; j-- > 0uz;) {
      ImGui::Checkbox((UNIQUE_LABEL(I) + std::to_string(j)).c_str(), &f[j]);
      ImGui::SameLine();
    }
    ImGui::TextUnformatted("?");
    d = static_cast<uint8_t>(f[7uz] << 7u | f[6uz] << 6u | f[5uz] << 5u |
                             f[4uz] << 4u | f[3uz] << 3u | f[2uz] << 2u |
                             f[1uz] << 1u | f[0uz] << 0u);
  }
  // Input register
  else if (!strcmp(labels[static_cast<size_t>(i)], "Input Register")) {
    static std::array<bool, CHAR_BIT> f{};
    for (auto j{size(f)}; j-- > 0uz;) {
      ImGui::Checkbox((UNIQUE_LABEL(I) + std::to_string(j)).c_str(), &f[j]);
      ImGui::SameLine();
    }
    ImGui::TextUnformatted("?");
    d = static_cast<uint8_t>(f[7uz] << 7u | f[6uz] << 6u | f[5uz] << 5u |
                             f[4uz] << 4u | f[3uz] << 3u | f[2uz] << 2u |
                             f[1uz] << 1u | f[0uz] << 0u);
  }
  // QoS
  else if (!strcmp(labels[static_cast<size_t>(i)], "Quality of Service")) {
    static constexpr uint8_t min{0u};
    static constexpr uint8_t max{100u};
    ImGui::SliderScalar(
      ("[%]##" + std::to_string(I)).c_str(), ImGuiDataType_U8, &d, &min, &max);
  }
  // Container levels
  else if (!strcmp(labels[static_cast<size_t>(i)], "Container 1 Level") ||
           !strcmp(labels[static_cast<size_t>(i)], "Container 2 Level") ||
           !strcmp(labels[static_cast<size_t>(i)], "Container 3 Level") ||
           !strcmp(labels[static_cast<size_t>(i)], "Container 4 Level") ||
           !strcmp(labels[static_cast<size_t>(i)], "Container 5 Level") ||
           !strcmp(labels[static_cast<size_t>(i)], "Container 6 Level") ||
           !strcmp(labels[static_cast<size_t>(i)], "Container 7 Level") ||
           !strcmp(labels[static_cast<size_t>(i)], "Container 8 Level") ||
           !strcmp(labels[static_cast<size_t>(i)], "Container 9 Level") ||
           !strcmp(labels[static_cast<size_t>(i)], "Container 10 Level") ||
           !strcmp(labels[static_cast<size_t>(i)], "Container 11 Level") ||
           !strcmp(labels[static_cast<size_t>(i)], "Container 12 Level")) {
    static constexpr uint8_t min{0u};
    static constexpr uint8_t max{100u};
    ImGui::SliderScalar(
      ("[%]##" + std::to_string(I)).c_str(), ImGuiDataType_U8, &d, &min, &max);
  }
  // Position (app:ext)
  else if (!strcmp(labels[static_cast<size_t>(i)], "Position (app:ext)")) {
    static uint16_t p{};
    ImGui::InputScalar(UNIQUE_LABEL(I), ImGuiDataType_U16, &p);
    p = std::clamp<uint16_t>(p, 0u, smath::pow(2u, I ? 8u : 11u) - 1u);
    if (p <= std::numeric_limits<uint8_t>::max()) d = static_cast<uint8_t>(p);
    else {
      auto const first{make_app_dyn_datagram(static_cast<uint8_t>(p >> 0u),
                                             static_cast<uint8_t>(i - 1))};
      std::ranges::copy(first, begin(datagram.bytes) + channel1_size);
      auto const second{make_app_dyn_datagram(static_cast<uint8_t>(p >> 8u),
                                              static_cast<uint8_t>(i - 1))};
      std::ranges::copy(second,
                        begin(datagram.bytes) + channel1_size + size(first));
      return;
    }
  }
  // Status and alarm messages
  else if (!strcmp(labels[static_cast<size_t>(i)],
                   "Status and Alarm Messages")) {
    static bool dv_related{};
    ImGui::Checkbox(("Related to DV##" + std::to_string(I)).c_str(),
                    &dv_related);
    if (dv_related) {
      static bool alarm{};
      ImGui::Checkbox(("Alarm##" + std::to_string(I)).c_str(), &alarm);
      static uint8_t dv{};
      ImGui::InputScalar(
        ("DV##" + std::to_string(I)).c_str(), ImGuiDataType_U8, &dv);
      dv = std::clamp<uint8_t>(dv, 0u, smath::pow(2u, 6u) - 1u);
      d = static_cast<uint8_t>(alarm << 7u | dv_related << 6u | dv);
    }
    // MOB
    else if (std::ranges::equal(mob_app_dyn_labels, labels)) {
      static int j{};
      ImGui::Combo(("##" + std::to_string(I)).c_str(),
                   &j,
                   data(mob_app_dyn_status_and_alarm_messages_labels),
                   ssize(mob_app_dyn_status_and_alarm_messages_labels));
      d = 0x80u | static_cast<uint8_t>(j);
    }
    // STAT
    else if (std::ranges::equal(stat_app_dyn_labels, labels)) {
      static int j{};
      ImGui::Combo(("##" + std::to_string(I)).c_str(),
                   &j,
                   data(stat_app_dyn_status_and_alarm_messages_labels),
                   ssize(stat_app_dyn_status_and_alarm_messages_labels));
      d = 0x80u | static_cast<uint8_t>(j);
    }
  }
  // Trip odometer
  else if (!strcmp(labels[static_cast<size_t>(i)], "Trip Odometer")) {
    ImGui::InputScalar(
      ("##" + std::to_string(I)).c_str(), ImGuiDataType_U8, &d);
  }
  // Maintenance interval
  else if (!strcmp(labels[static_cast<size_t>(i)], "Maintenance Interval")) {
    ImGui::InputScalar(
      ("##" + std::to_string(I)).c_str(), ImGuiDataType_U8, &d);
  }
  // Temperature
  else if (!strcmp(labels[static_cast<size_t>(i)], "Temperature")) {
    static constexpr int16_t min{-50};
    static constexpr int16_t max{205};
    static int16_t temp{};
    ImGui::SliderScalar(("[°C]##" + std::to_string(I)).c_str(),
                        ImGuiDataType_S16,
                        &temp,
                        &min,
                        &max);
    d = static_cast<uint8_t>(ztl::lerp<int32_t>(temp, min, max, 0, 255));
  }
  // Direction status byte
  else if (!strcmp(labels[static_cast<size_t>(i)], "Direction Status Byte")) {
    static std::array<bool, CHAR_BIT> f{};
    ImGui::Checkbox(("Direction##" + std::to_string(I)).c_str(), &f[0uz]);
    ImGui::Checkbox(("East-West##" + std::to_string(I)).c_str(), &f[1uz]);
    ImGui::Checkbox(("Direction Control##" + std::to_string(I)).c_str(),
                    &f[2uz]);
    ImGui::Checkbox(("Direction Change##" + std::to_string(I)).c_str(),
                    &f[3uz]);
    ImGui::Checkbox(("Hide UI##" + std::to_string(I)).c_str(), &f[4uz]);
    ImGui::Checkbox(("East-West Inverted##" + std::to_string(I)).c_str(),
                    &f[5uz]);
    d = static_cast<uint8_t>(f[5uz] << 5u | f[4uz] << 4u | f[3uz] << 3u |
                             f[2uz] << 2u | f[1uz] << 1u | f[0uz] << 0u);
  }
  // Control deviation
  else if (!strcmp(labels[static_cast<size_t>(i)], "Control Deviation")) {
    static constexpr int8_t min{std::numeric_limits<int8_t>::min()};
    static constexpr int8_t max{std::numeric_limits<int8_t>::max()};
    ImGui::SliderScalar(
      ("##" + std::to_string(I)).c_str(), ImGuiDataType_S8, &d, &min, &max);
  }
  // Track voltage
  else if (!strcmp(labels[static_cast<size_t>(i)], "Track Voltage")) {
    static constexpr float min{5.0f};
    static constexpr float max{30.5f};
    static float v{min};
    ImGui::SliderFloat(
      ("[V]##" + std::to_string(I)).c_str(), &v, min, max, "%.1f");
    d = static_cast<uint8_t>((v - 5.0f) / 0.1f);
  }
  // Stopping distance
  else if (!strcmp(labels[static_cast<size_t>(i)], "Stopping Distance")) {
    static constexpr uint8_t min{0u};
    static constexpr uint8_t max{255u};
    ImGui::SliderScalar("[m]",
                        ImGuiDataType_U8,
                        &d,
                        &min,
                        &max,
                        app_dyn_stopping_distance_labels[d]);
  }
  // Reserved
  else if (strstr(labels[static_cast<size_t>(i)], "Reserved")) {
    ImGui::InputScalar(
      ("D##" + std::to_string(I)).c_str(), ImGuiDataType_U8, &d);
  }
  // Neither
  else
    return;
  auto it{std::ranges::find(
    begin(datagram.bytes) + channel1_size, end(datagram.bytes), 0u)};
  std::ranges::copy(make_app_dyn_datagram(d, static_cast<uint8_t>(i - 1)), it);
  if (it == cbegin(datagram.bytes) + channel1_size)
    return app_dyn<1uz>(datagram, labels);
}

// Loco app:xpom
void app_xpom(State::Datagram& datagram) {
  ImGui::SeparatorText("Parameters");
  static constexpr std::array ss{
    "SS = 00 - ID8", "SS = 01 - ID9", "SS = 10 - ID10", "SS = 11 - ID11"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(ss), ssize(ss));
  static std::array<uint8_t, 4uz> cv_values{};
  ImGui::InputScalarN(
    "CV Values", ImGuiDataType_U8, data(cv_values), ssize(cv_values));
  std::ranges::copy(make_app_xpom_datagram(static_cast<uint8_t>(i), cv_values),
                    begin(datagram.bytes) + channel1_size);
}

// Loco app:CV-auto
void app_cv_auto(State::Datagram& datagram) {
  ImGui::SeparatorText("Parameters");
  static uint32_t cv_addr{0u};
  ImGui::InputScalar("CV Address", ImGuiDataType_U32, &cv_addr);
  cv_addr = std::clamp(cv_addr, 0u, (1u << 24u) - 1u);
  static uint8_t cv_value{0u};
  ImGui::InputScalar("CV Value", ImGuiDataType_U8, &cv_value);
  std::ranges::copy(make_app_cv_auto_datagram(cv_addr, cv_value),
                    begin(datagram.bytes) + channel1_size);
}

// Loco app:block
void app_block(State::Datagram&) {
  ImGui::SeparatorText("Parameters");
  ImGui::TextUnformatted("\\todo");
}

} // namespace loco

////////////////////////////////////////////////////////////////////////////////

namespace accessory {

// clang-format off
void accessory(State::Datagram& datagram);
  void channel1(State::Datagram& datagram);
    void app_srq(State::Datagram& datagram);
  void channel2(State::Datagram& datagram);
    void app_pom(State::Datagram& datagram);
    void app_stat4(State::Datagram& datagram);
    void app_stat1(State::Datagram& datagram);
    void app_time(State::Datagram& datagram);
    void app_error(State::Datagram& datagram);
    void app_dyn(State::Datagram& datagram);
    void app_xpom(State::Datagram& datagram);
    void app_test(State::Datagram& datagram);
    void app_block(State::Datagram& datagram);
// clang-format on

// Accessory
void accessory(State::Datagram& datagram) {
  datagram.addr = {.value = 12u,
                   .type = dcc::Address::BasicAccessory}; // Default
  channel1(datagram);
  channel2(datagram);
  if (std::ranges::all_of(datagram.bytes, [](auto b) { return !b; })) return;
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Datagrams")) state.datagrams.push_back(datagram);
}

// Accessory channel1
void channel1(State::Datagram& datagram) {
  ImGui::SeparatorText("Channel 1");
  static constexpr std::array apps{"", "app:srq"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(apps), ssize(apps));
  if (!strcmp(apps[static_cast<size_t>(i)], "app:srq")) app_srq(datagram);
}

// Accessory app:srq
void app_srq(State::Datagram& datagram) {
  ImGui::SeparatorText("Parameters");
  static constexpr std::array types{"Basic Accessory", "Extended Accessory"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(types), ssize(types));
  static dcc::Address::value_type addr{datagram.addr};
  ImGui::InputScalar("Address", ImGuiDataType_U16, &addr);
  addr = std::clamp<dcc::Address::value_type>(addr, 0u, 2047u);
  datagram.addr = {.value = addr,
                   .type = i ? dcc::Address::ExtendedAccessory
                             : dcc::Address::BasicAccessory};
  std::ranges::copy(make_app_srq_datagram(datagram.addr),
                    begin(datagram.bytes));
}

// Accessory channel2
void channel2(State::Datagram& datagram) {
  ImGui::SeparatorText("Channel 2");
  static constexpr std::array apps{
    "",
    "app:pom",
    "app:stat4",
    "app:stat1",
    "app:time",
    "app:error",
    "app:dyn",
    "app:xpom",
    // "app:test",
    // "app:block"
  };
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(apps), ssize(apps));
  if (!strcmp(apps[static_cast<size_t>(i)], "app:pom")) app_pom(datagram);
  else if (!strcmp(apps[static_cast<size_t>(i)], "app:stat4"))
    app_stat4(datagram);
  else if (!strcmp(apps[static_cast<size_t>(i)], "app:stat1"))
    app_stat1(datagram);
  else if (!strcmp(apps[static_cast<size_t>(i)], "app:time"))
    app_time(datagram);
  else if (!strcmp(apps[static_cast<size_t>(i)], "app:error"))
    app_error(datagram);
  else if (!strcmp(apps[static_cast<size_t>(i)], "app:dyn")) app_dyn(datagram);
  else if (!strcmp(apps[static_cast<size_t>(i)], "app:xpom"))
    app_xpom(datagram);
  else if (!strcmp(apps[static_cast<size_t>(i)], "app:test"))
    app_test(datagram);
  else if (!strcmp(apps[static_cast<size_t>(i)], "app:block"))
    app_block(datagram);
}

// Accessory app:pom
void app_pom(State::Datagram& datagram) { return loco::app_pom(datagram); }

// Accessory app:stat4
void app_stat4(State::Datagram& datagram) {
  ImGui::SeparatorText("Parameters");
  static std::array<bool, 4uz> rs{};
  for (auto i{size(rs)}; i-- > 0uz;) {
    ImGui::Checkbox(UNIQUE_LABEL(i), &rs[i]);
    ImGui::SameLine();
  }
  ImGui::TextUnformatted("Outputs R4-R1");
  uint8_t cv33{};
  for (auto i{size(rs)}; i-- > 0uz;)
    cv33 = static_cast<uint8_t>(cv33 | 1u << ((2u * i) + (1u - rs[i])));
  std::ranges::copy(make_app_stat4_datagram(cv33),
                    begin(datagram.bytes) + channel1_size);
}

// Accessory app:stat1
void app_stat1(State::Datagram& datagram) {
  ImGui::SeparatorText("Parameters");
  static constexpr std::array types{"Basic Accessory", "Extended Accessory"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(types), ssize(types));
  datagram.addr.type =
    i ? dcc::Address::ExtendedAccessory : dcc::Address::BasicAccessory;
  static bool bit6{};
  ImGui::Checkbox("Initial State Matches Last Received", &bit6);
  static bool bit5{};
  ImGui::Checkbox("Returned Aspect Based on Feedback", &bit5);
  static uint8_t aspect{};
  ImGui::InputScalar("Aspect", ImGuiDataType_U8, &aspect);
  aspect = std::clamp<uint8_t>(
    aspect, 0u, static_cast<uint8_t>(smath::pow(2u, i ? 8u : 5u) - 1u));
  auto const aspect_low_bits{aspect & 0b1'1111u};
  auto const first{make_app_stat1_datagram(
    static_cast<uint8_t>(bit6 << 6u | bit5 << 5u | aspect_low_bits))};
  std::ranges::copy(first, begin(datagram.bytes) + channel1_size);
  if (!strcmp(types[static_cast<size_t>(i)], "Extended Accessory") &&
      aspect != aspect_low_bits) {
    auto const aspect_high_bits{static_cast<uint8_t>(aspect >> 5u)};
    auto const second{
      make_app_stat1_datagram(static_cast<uint8_t>(0x80u | aspect_high_bits))};
    std::ranges::copy(second,
                      begin(datagram.bytes) + channel1_size + size(first));
  }
}

// Accessory app:time
void app_time(State::Datagram& datagram) {
  ImGui::SeparatorText("Parameters");
  static constexpr std::array types{"0.1s", "1s"};
  static int i{};
  ImGui::Combo("Resolution", &i, data(types), ssize(types));
  static uint8_t time{};
  ImGui::InputScalar("Time", ImGuiDataType_U8, &time);
  time = std::clamp<uint8_t>(time, 0u, smath::pow(2u, 7u) - 1u);
  std::ranges::copy(make_app_time_datagram(i, time),
                    begin(datagram.bytes) + channel1_size);
}

// Accessory app:error
void app_error(State::Datagram& datagram) {
  ImGui::SeparatorText("Parameters");
  static bool additional_errors{};
  ImGui::Checkbox("Additional Errors", &additional_errors);
  static int i{};
  ImGui::Combo("Code", &i, data(app_error_labels), ssize(app_error_labels));
  app::Error::Code code{additional_errors ? app::Error::AdditionalErrors
                                          : app::Error::None};
  if (!strcmp(app_error_labels[static_cast<size_t>(i)], "None"))
    code = static_cast<app::Error::Code>(code | app::Error::None);
  else if (!strcmp(app_error_labels[static_cast<size_t>(i)], "Invalid Command"))
    code = static_cast<app::Error::Code>(code | app::Error::InvalidCommand);
  else if (!strcmp(app_error_labels[static_cast<size_t>(i)], "Overcurrent"))
    code = static_cast<app::Error::Code>(code | app::Error::Overcurrent);
  else if (!strcmp(app_error_labels[static_cast<size_t>(i)], "Undervoltage"))
    code = static_cast<app::Error::Code>(code | app::Error::Undervoltage);
  else if (!strcmp(app_error_labels[static_cast<size_t>(i)], "Fuse"))
    code = static_cast<app::Error::Code>(code | app::Error::Fuse);
  else if (!strcmp(app_error_labels[static_cast<size_t>(i)], "Overtemperature"))
    code = static_cast<app::Error::Code>(code | app::Error::Overtemperature);
  else if (!strcmp(app_error_labels[static_cast<size_t>(i)], "Feedback"))
    code = static_cast<app::Error::Code>(code | app::Error::Feedback);
  else if (!strcmp(app_error_labels[static_cast<size_t>(i)],
                   "Manual Operation"))
    code = static_cast<app::Error::Code>(code | app::Error::ManualOperation);
  else if (!strcmp(app_error_labels[static_cast<size_t>(i)], "Signal"))
    code = static_cast<app::Error::Code>(code | app::Error::Signal);
  else if (!strcmp(app_error_labels[static_cast<size_t>(i)], "Servo"))
    code = static_cast<app::Error::Code>(code | app::Error::Servo);
  else if (!strcmp(app_error_labels[static_cast<size_t>(i)], "Internal"))
    code = static_cast<app::Error::Code>(code | app::Error::Internal);
  std::ranges::copy(make_app_error_datagram(code),
                    begin(datagram.bytes) + channel1_size);
}

// Accessory app:dyn
void app_dyn(State::Datagram& datagram) {
  return loco::app_dyn<0uz>(datagram, stat_app_dyn_labels);
}

// Accessory app:xpom
void app_xpom(State::Datagram& datagram) { return loco::app_xpom(datagram); }

// Accessory app:test
void app_test(State::Datagram&) {
  ImGui::SeparatorText("Parameters");
  ImGui::TextUnformatted("\\todo");
}

// Accessory app:block
void app_block(State::Datagram&) {
  ImGui::SeparatorText("Parameters");
  ImGui::TextUnformatted("\\todo");
}

} // namespace accessory

namespace automatic_logon {

void automatic_logon(State::Datagram& datagram) {
  datagram.addr = {.value = 254u,
                   .type = dcc::Address::AutomaticLogon}; // Default
  ImGui::TextUnformatted("\\todo");
}

} // namespace automatic_logon

} // namespace

// Datagram builder window
void datagram_builder() {
  if (!state.windows.show_datagram_builder) return;

  if (ImGui::Begin("Datagram Builder",
                   &state.windows.show_datagram_builder,
                   ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::SeparatorText("Type");
    static constexpr std::array types{
      "",
      "Broadcast",
      "Loco",
      "Accessory",
      // "Automatic Logon"
    };
    static int i{};
    ImGui::Combo(UNIQUE_LABEL(), &i, data(types), ssize(types));
    if (State::Datagram datagram{};
        !strcmp(types[static_cast<size_t>(i)], "Broadcast"))
      broadcast::broadcast(datagram);
    else if (!strcmp(types[static_cast<size_t>(i)], "Loco"))
      loco::loco(datagram);
    else if (!strcmp(types[static_cast<size_t>(i)], "Accessory"))
      accessory::accessory(datagram);
    else if (!strcmp(types[static_cast<size_t>(i)], "Automatic Logon"))
      automatic_logon::automatic_logon(datagram);
  }
  ImGui::End();
}
