#include "datagram_builder.hpp"
#include <imgui.h>
#include "utility.hpp"

namespace {

using namespace dcc::bidi;

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

// clang-format off
void broadcast(State& state, Datagram<>& datagram);
  void channel2(State& state, Datagram<>& datagram);
    void app_tos(State& state, Datagram<>& datagram);
// clang-format on

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
  ImGui::TextUnformatted("\\todo");
}

} // namespace broadcast

////////////////////////////////////////////////////////////////////////////////

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
  ImGui::TextUnformatted("\\todo");
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
  static uint8_t cv{};
  ImGui::InputScalar("CV", ImGuiDataType_U8, &cv);
  auto const dg{encode_datagram(make_datagram<Bits::_12>(0u, cv))};
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
  ImGui::TextUnformatted("\\todo");
}

//
void app_cv_auto(State& state, Datagram<>& datagram) {
  ImGui::SeparatorText("Parameters");
  ImGui::TextUnformatted("\\todo");
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

////////////////////////////////////////////////////////////////////////////////

namespace tab {

// clang-format off
void tab(State::Packet& packet, size_t i);
  void description(State::Packet& packet);
  void data(State::Packet& packet);
  void plot(State::Packet& packet);
// clang-format on

} // namespace tab

} // namespace

void datagram_builder(State& state) {
  if (!state.windows.show_datagram_builder) return;

  if (state.loco_datagrams.empty()) {
    auto const dg{::app::adr_high(3u, false)};
    Datagram<> datagram{};
    std::ranges::copy(dg, begin(datagram));
    state.loco_datagrams.push_back({.bytes = datagram});
  }

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
