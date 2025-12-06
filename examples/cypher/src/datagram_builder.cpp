#include "datagram_builder.hpp"
#include <imgui.h>
#include "utility.hpp"

namespace {

using namespace dcc::bidi;

namespace broadcast {

// clang-format off
void broadcast(State& state, Datagram<>& datagram);
  void channel2(State& state, Datagram<>& datagram);
    void app_tos(State& state, Datagram<>& datagram);
// clang-format on

} // namespace broadcast

namespace loco {

// clang-format off
void loco(State& state, Datagram<>& datagram);
  void channel1(State& state, Datagram<>& datagram);
    void app_adr_high(State& state, Datagram<>& datagram);
    void app_adr_low(State& state, Datagram<>& datagram);
    void app_adr_info1(State& state, Datagram<>& datagram);
  void channel2(State& state, Datagram<>& datagram);
    void app_pom(State& state, Datagram<>& datagram);
    void app_ext(State& state, Datagram<>& datagram);
    void app_info(State& state, Datagram<>& datagram);
    void app_dyn(State& state, Datagram<>& datagram);
    void app_xpom(State& state, Datagram<>& datagram);
    void app_cv_auto(State& state, Datagram<>& datagram);
    void app_block(State& state, Datagram<>& datagram);
// clang-format on

} // namespace loco

namespace app {

//
auto adr_high(dcc::Address::value_type addr, bool consist) {
  if (consist && addr < 128u)
    return encode_datagram(make_datagram<Bits::_12>(1u, 0b0110'0000u));
  else
    return encode_datagram(make_datagram<Bits::_12>(
      1u, addr < 128u ? 0u : 0x80u | (addr & 0x3F00u) >> 8u));
}

//
auto adr_low(dcc::Address::value_type addr, bool consist, bool r) {
  if (consist && addr < 128u)
    return encode_datagram(make_datagram<Bits::_12>(
      2u, static_cast<uint8_t>(r << 7u) | (addr & 0x007Fu)));
  else return encode_datagram(make_datagram<Bits::_12>(2u, addr & 0x00FFu));
}

} // namespace app

namespace broadcast {

//
void broadcast(State& state, Datagram<>& datagram) {
  channel2(state, datagram);
  if (std::ranges::all_of(datagram, [](auto b) { return !b; })) return;
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Datagrams"))
    state.loco_datagrams.push_back({.bytes = datagram});
}

//
void channel2(State& state, Datagram<>& datagram) {
  ImGui::SeparatorText("Channel 2");
  static constexpr std::array ch2_datagrams{"", "app:tos"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(ch2_datagrams), ssize(ch2_datagrams));
  if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:tos"))
    app_tos(state, datagram);
}

//
void app_tos(State& state, Datagram<>& datagram) {
  ImGui::SeparatorText("Parameters");
  static dcc::Address::value_type addr{3};
  ImGui::InputScalar("Address", ImGuiDataType_U16, &addr);
  addr = std::clamp<dcc::Address::value_type>(addr, 1u, 10239u);
  static bool consist{};
  ImGui::Checkbox("Consist", &consist);
  static bool r{};
  if (consist) ImGui::Checkbox("R", &r);
  static constexpr uint8_t min{0u};
  static constexpr uint8_t max{30u};
  static uint8_t s{};
  ImGui::SliderScalar("[s]", ImGuiDataType_U8, &s, &min, &max);
  auto const adr_high{app::adr_high(addr, consist)};
  auto it{std::copy(
    cbegin(adr_high), cend(adr_high), begin(datagram) + channel1_size)};
  auto const adr_low{app::adr_low(addr, consist, r)};
  it = std::copy(cbegin(adr_low), cend(adr_low), it);
  auto const time{encode_datagram(make_datagram<Bits::_12>(14u, s))};
  std::copy(cbegin(time), cend(time), it);
}

} // namespace broadcast

////////////////////////////////////////////////////////////////////////////////

namespace loco {

//
void loco(State& state, Datagram<>& datagram) {
  channel1(state, datagram);
  channel2(state, datagram);
  if (std::ranges::all_of(datagram, [](auto b) { return !b; })) return;
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Datagrams"))
    state.loco_datagrams.push_back({.bytes = datagram});
}

//
void channel1(State& state, Datagram<>& datagram) {
  ImGui::SeparatorText("Channel 1");
  static constexpr std::array ch1_datagrams{
    "", "app:adr_high", "app:adr_low", "app:info1"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(ch1_datagrams), ssize(ch1_datagrams));
  if (!strcmp(ch1_datagrams[static_cast<size_t>(i)], "app:adr_high"))
    app_adr_high(state, datagram);
  else if (!strcmp(ch1_datagrams[static_cast<size_t>(i)], "app:adr_low"))
    app_adr_low(state, datagram);
  else if (!strcmp(ch1_datagrams[static_cast<size_t>(i)], "app:info1"))
    app_adr_info1(state, datagram);
}

//
void app_adr_high(State& state, Datagram<>& datagram) {
  ImGui::SeparatorText("Parameters");
  static dcc::Address::value_type addr{3};
  ImGui::InputScalar("Address", ImGuiDataType_U16, &addr);
  addr = std::clamp<dcc::Address::value_type>(addr, 1u, 10239u);
  static bool consist{};
  ImGui::Checkbox("Consist", &consist);
  auto const dg{app::adr_high(addr, consist)};
  std::ranges::copy(dg, begin(datagram));
}

//
void app_adr_low(State& state, Datagram<>& datagram) {
  ImGui::SeparatorText("Parameters");
  static dcc::Address::value_type addr{3};
  ImGui::InputScalar("Address", ImGuiDataType_U16, &addr);
  addr = std::clamp<dcc::Address::value_type>(addr, 1u, 10239u);
  static bool consist{};
  ImGui::Checkbox("Consist", &consist);
  static bool r{};
  if (consist) ImGui::Checkbox("R", &r);
  auto const dg{app::adr_low(addr, consist, r)};
  std::ranges::copy(dg, begin(datagram));
}

//
void app_adr_info1(State& state, Datagram<>& datagram) {
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
void channel2(State& state, Datagram<>& datagram) {
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
    app_pom(state, datagram);
  else if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:ext"))
    app_ext(state, datagram);
  else if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:info"))
    app_info(state, datagram);
  else if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:dyn"))
    app_dyn(state, datagram);
  else if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:xpom"))
    app_xpom(state, datagram);
  else if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:CV-auto"))
    app_cv_auto(state, datagram);
  else if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:block"))
    app_block(state, datagram);
}

//
void app_pom(State& state, Datagram<>& datagram) {
  ImGui::SeparatorText("Parameters");
  static uint8_t cv_value{};
  ImGui::InputScalar("CV value", ImGuiDataType_U8, &cv_value);
  auto const dg{encode_datagram(make_datagram<Bits::_12>(0u, cv_value))};
  std::ranges::copy(dg, begin(datagram) + 2);
}

//
void app_ext(State& state, Datagram<>& datagram) {
  ImGui::SeparatorText("Parameters");
  ImGui::TextUnformatted("\\todo");
}

//
void app_info(State& state, Datagram<>& datagram) {
  ImGui::SeparatorText("Parameters");
  ImGui::TextUnformatted("\\todo");
}

//
void app_dyn(State& state, Datagram<>& datagram) {
  ImGui::SeparatorText("Parameters");
  ImGui::TextUnformatted("\\todo");
}

//
void app_xpom(State& state, Datagram<>& datagram) {
  ImGui::SeparatorText("Parameters");
  static constexpr std::array ss{
    "SS = 00 - ID8", "SS = 01 - ID9", "SS = 10 - ID10", "SS = 11 - ID11"};
  static int i{};
  ImGui::Combo(UNIQUE_LABEL(), &i, data(ss), ssize(ss));
  static std::array<uint16_t, 4uz> cv_values{};
  ImGui::InputScalarN(
    "CV Values", ImGuiDataType_U8, data(cv_values), ssize(cv_values));
  auto const dg{encode_datagram(make_datagram<Bits::_36>(
    0b10u << 2u | i,
    static_cast<uint32_t>(cv_values[0uz] << 24u | cv_values[1uz] << 16u |
                          cv_values[2uz] << 8u | cv_values[3uz] << 0u)))};
  std::ranges::copy(dg, begin(datagram) + channel1_size);
}

//
void app_cv_auto(State& state, Datagram<>& datagram) {
  ImGui::SeparatorText("Parameters");
  static uint32_t cv_addr{0u};
  ImGui::InputScalar("CV Address", ImGuiDataType_U32, &cv_addr);
  cv_addr = std::clamp<dcc::Address::value_type>(cv_addr, 0u, (1u << 24u) - 1u);
  static uint8_t cv_value{0u};
  ImGui::InputScalar("CV Value", ImGuiDataType_U8, &cv_value);
  auto const dg{encode_datagram(
    make_datagram<Bits::_36>(12u, cv_addr << 8u | cv_value << 0u))};
  std::ranges::copy(dg, begin(datagram) + channel1_size);
}

//
void app_block(State& state, Datagram<>& datagram) {
  ImGui::SeparatorText("Parameters");
  ImGui::TextUnformatted("\\todo");
}

} // namespace loco

////////////////////////////////////////////////////////////////////////////////

namespace accessory {

//
void accessory(State& state, Datagram<>& datagram) {
  ImGui::TextUnformatted("\\todo");
}

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
    else if (Datagram<> datagram{};
             !strcmp(types[static_cast<size_t>(i)], "Loco"))
      loco::loco(state, datagram);
    else if (!strcmp(types[static_cast<size_t>(i)], "Accessory"))
      accessory::accessory(state, datagram);
    // else if (!strcmp(types[static_cast<size_t>(i)], "Automatic Logon"))
    //   ;
  }
  ImGui::End();
}
