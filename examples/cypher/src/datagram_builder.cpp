#include "datagram_builder.hpp"
#include <imgui.h>
#include "utility.hpp"

namespace {

using namespace dcc::bidi;

namespace broadcast {

// clang-format off
void broadcast(State& state, Datagram<>& datagram);
  void channel2(Datagram<>& datagram);
    void app_search(Datagram<>& datagram);
// clang-format on

} // namespace broadcast

namespace loco {

// clang-format off
void loco(State& state, Datagram<>& datagram);
  void channel1(Datagram<>& datagram);
    void app_adr_high(Datagram<>& datagram);
    void app_adr_low(Datagram<>& datagram);
    void app_adr_info1(Datagram<>& datagram);
  void channel2(Datagram<>& datagram);
    void app_pom(Datagram<>& datagram);
    void app_ext(Datagram<>& datagram);
    void app_info(Datagram<>& datagram);
    void app_dyn(Datagram<>& datagram);
    void app_xpom(Datagram<>& datagram);
    void app_cv_auto(Datagram<>& datagram);
    void app_block(Datagram<>& datagram);
// clang-format on

} // namespace loco

namespace broadcast {

//
void broadcast(State& state, Datagram<>& datagram) {
  channel2(datagram);
  if (std::ranges::all_of(datagram, [](auto b) { return !b; })) return;
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Datagrams"))
    state.datagrams.push_back(
      {.addr = {.type = dcc::Address::Broadcast}, .bytes = datagram});
}

//
void channel2(Datagram<>& datagram) {
  ImGui::SeparatorText("Channel 2");
  static constexpr std::array ch2_datagrams{"", "app:search"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(ch2_datagrams), ssize(ch2_datagrams));
  if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:search"))
    app_search(datagram);
}

//
void app_search(Datagram<>& datagram) {
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
  std::ranges::copy(dcc::make_app_search_datagram(addr, r << 7u | consist, s),
                    begin(datagram) + channel1_size);
}

} // namespace broadcast

////////////////////////////////////////////////////////////////////////////////

namespace loco {

//
void loco(State& state, Datagram<>& datagram) {
  channel1(datagram);
  channel2(datagram);
  if (std::ranges::all_of(datagram, [](auto b) { return !b; })) return;
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Datagrams"))
    state.datagrams.push_back(
      {.addr = {.value = 3u, .type = dcc::Address::BasicLoco},
       .bytes = datagram});
}

//
void channel1(Datagram<>& datagram) {
  ImGui::SeparatorText("Channel 1");
  static constexpr std::array ch1_datagrams{
    "", "app:adr_high", "app:adr_low", "app:info1"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(ch1_datagrams), ssize(ch1_datagrams));
  if (!strcmp(ch1_datagrams[static_cast<size_t>(i)], "app:adr_high"))
    app_adr_high(datagram);
  else if (!strcmp(ch1_datagrams[static_cast<size_t>(i)], "app:adr_low"))
    app_adr_low(datagram);
  else if (!strcmp(ch1_datagrams[static_cast<size_t>(i)], "app:info1"))
    app_adr_info1(datagram);
}

//
void app_adr_high(Datagram<>& datagram) {
  ImGui::SeparatorText("Parameters");
  static dcc::Address::value_type addr{3};
  ImGui::InputScalar("Address", ImGuiDataType_U16, &addr);
  addr = std::clamp<dcc::Address::value_type>(addr, 1u, 10239u);
  static bool consist{};
  ImGui::Checkbox("Consist", &consist);
  std::ranges::copy(dcc::make_app_adr_high_datagram(addr, consist),
                    begin(datagram));
}

//
void app_adr_low(Datagram<>& datagram) {
  ImGui::SeparatorText("Parameters");
  static dcc::Address::value_type addr{3};
  ImGui::InputScalar("Address", ImGuiDataType_U16, &addr);
  addr = std::clamp<dcc::Address::value_type>(addr, 1u, 10239u);
  static bool consist{};
  ImGui::Checkbox("Consist", &consist);
  static bool r{};
  if (consist) ImGui::Checkbox("Reversed", &r);
  std::ranges::copy(dcc::make_app_adr_low_datagram(addr, r << 7u | consist),
                    begin(datagram));
}

//
void app_adr_info1(Datagram<>& datagram) {
  ImGui::SeparatorText("Parameters");
  static bool track_polarity{};
  ImGui::Checkbox("Track polarity", &track_polarity);
  static bool direction{};
  ImGui::Checkbox("Direction", &direction);
  static bool driving{};
  ImGui::Checkbox("Driving", &driving);
  static bool consist{};
  ImGui::Checkbox("Consist", &consist);
  static bool addr_request{};
  ImGui::Checkbox("Addressing request", &addr_request);
  auto const dg{encode_datagram(make_datagram<Bits::_12>(
    3u,
    static_cast<uint8_t>(addr_request << 4u | consist << 3u | driving << 2u |
                         direction << 1u | track_polarity << 0u)))};
  std::ranges::copy(dg, begin(datagram));
}

//
void channel2(Datagram<>& datagram) {
  ImGui::SeparatorText("Channel 2");
  static constexpr std::array ch2_datagrams{"",
                                            "app:pom",
                                            "app:ext",
                                            "app:info",
                                            "app:dyn",
                                            "app:xpom",
                                            "app:CV-auto",
                                            "app:block"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(ch2_datagrams), ssize(ch2_datagrams));
  if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:pom"))
    app_pom(datagram);
  else if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:ext"))
    app_ext(datagram);
  else if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:info"))
    app_info(datagram);
  else if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:dyn"))
    app_dyn(datagram);
  else if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:xpom"))
    app_xpom(datagram);
  else if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:CV-auto"))
    app_cv_auto(datagram);
  else if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:block"))
    app_block(datagram);
}

//
void app_pom(Datagram<>& datagram) {
  ImGui::SeparatorText("Parameters");
  static uint8_t cv_value{};
  ImGui::InputScalar("CV value", ImGuiDataType_U8, &cv_value);
  std::ranges::copy(dcc::make_app_pom_datagram(cv_value), begin(datagram) + 2);
}

//
void app_ext(Datagram<>& datagram) {
  ImGui::SeparatorText("Parameters");
  ImGui::TextUnformatted("\\todo");
}

//
void app_info(Datagram<>& datagram) {
  ImGui::SeparatorText("Parameters");
  ImGui::TextUnformatted("\\todo");
}

//
void app_dyn(Datagram<>& datagram) {
  ImGui::SeparatorText("Parameters");
  ImGui::TextUnformatted("\\todo");
}

//
void app_xpom(Datagram<>& datagram) {
  ImGui::SeparatorText("Parameters");
  static constexpr std::array ss{
    "SS = 00 - ID8", "SS = 01 - ID9", "SS = 10 - ID10", "SS = 11 - ID11"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(ss), ssize(ss));
  static std::array<uint8_t, 4uz> cv_values{};
  ImGui::InputScalarN(
    "CV Values", ImGuiDataType_U8, data(cv_values), ssize(cv_values));
  std::ranges::copy(
    dcc::make_app_xpom_datagram(static_cast<uint8_t>(i), cv_values),
    begin(datagram) + channel1_size);
}

//
void app_cv_auto(Datagram<>& datagram) {
  ImGui::SeparatorText("Parameters");
  static uint32_t cv_addr{0u};
  ImGui::InputScalar("CV Address", ImGuiDataType_U32, &cv_addr);
  cv_addr = std::clamp(cv_addr, 0u, (1u << 24u) - 1u);
  static uint8_t cv_value{0u};
  ImGui::InputScalar("CV Value", ImGuiDataType_U8, &cv_value);
  std::ranges::copy(dcc::make_app_cv_auto_datagram(cv_addr, cv_value),
                    begin(datagram) + channel1_size);
}

//
void app_block(Datagram<>& datagram) {
  ImGui::SeparatorText("Parameters");
  ImGui::TextUnformatted("\\todo");
}

} // namespace loco

////////////////////////////////////////////////////////////////////////////////

namespace accessory {

// clang-format off
void accessory(State& state, Datagram<>& datagram);
  void channel1(Datagram<>& datagram);
  void channel2(Datagram<>& datagram);
    void app_pom(Datagram<>& datagram);
    void app_stat4(Datagram<>& datagram);
    void app_stat1(Datagram<>& datagram);
    void app_time(Datagram<>& datagram);
    void app_error(Datagram<>& datagram);
    void app_dyn(Datagram<>& datagram);
    void app_xpom(Datagram<>& datagram);
    void app_test(Datagram<>& datagram);
    void app_block(Datagram<>& datagram);
// clang-format on

//
void accessory(State& state, Datagram<>& datagram) {
  channel1(datagram);
  channel2(datagram);
  if (std::ranges::all_of(datagram, [](auto b) { return !b; })) return;
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Datagrams"))
    state.datagrams.push_back(
      {.addr = {.value = 3u, .type = dcc::Address::ExtendedAccessory},
       .bytes = datagram});
}

//
void channel1(Datagram<>& datagram) {
  ImGui::SeparatorText("Channel 1");
  static constexpr std::array ch1_datagrams{"", "app:srq"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(ch1_datagrams), ssize(ch1_datagrams));
}

//
void channel2(Datagram<>& datagram) {
  ImGui::SeparatorText("Channel 2");
  static constexpr std::array ch2_datagrams{"",
                                            "app:pom",
                                            "app:stat4",
                                            "app:stat1",
                                            "app:time",
                                            "app:error",
                                            "app:dyn",
                                            "app:xpom",
                                            "app:test",
                                            "app:block"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(ch2_datagrams), ssize(ch2_datagrams));
  if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:pom"))
    app_pom(datagram);
  else if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:stat4"))
    app_stat4(datagram);
  else if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:stat1"))
    app_stat1(datagram);
  else if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:time"))
    app_time(datagram);
  else if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:error"))
    app_error(datagram);
  else if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:dyn"))
    app_dyn(datagram);
  else if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:xpom"))
    app_xpom(datagram);
  else if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:test"))
    app_test(datagram);
  else if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:block"))
    app_block(datagram);
}

//
void app_pom(Datagram<>& datagram) { return loco::app_pom(datagram); }

//
void app_stat4(Datagram<>& datagram) {}

//
void app_stat1(Datagram<>& datagram) {}

//
void app_time(Datagram<>& datagram) {}

//
void app_error(Datagram<>& datagram) {}

//
void app_dyn(Datagram<>& datagram) {}

//
void app_xpom(Datagram<>& datagram) { return loco::app_xpom(datagram); }

//
void app_test(Datagram<>& datagram) {}

//
void app_block(Datagram<>& datagram) {}

} // namespace accessory

} // namespace

void datagram_builder(State& state) {
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
    if (Datagram<> datagram{};
        !strcmp(types[static_cast<size_t>(i)], "Broadcast"))
      broadcast::broadcast(state, datagram);
    else if (!strcmp(types[static_cast<size_t>(i)], "Loco"))
      loco::loco(state, datagram);
    else if (!strcmp(types[static_cast<size_t>(i)], "Accessory"))
      accessory::accessory(state, datagram);
    // else if (!strcmp(types[static_cast<size_t>(i)], "Automatic Logon"))
    //   ;
  }
  ImGui::End();
}
