#include "datagram_builder.hpp"
#include <imgui.h>
#include "utility.hpp"

namespace {

namespace loco {

// clang-format off
void loco(State& state, dcc::bidi::Datagram<>& datagram);
  void channel1(State& state, dcc::bidi::Datagram<>& datagram);
    void app_adr_high(State& state, dcc::bidi::Datagram<>& datagram);
    void app_adr_low(State& state, dcc::bidi::Datagram<>& datagram);
    void app_adr_info1(State& state, dcc::bidi::Datagram<>& datagram);
  void channel2(State& state, dcc::bidi::Datagram<>& datagram);
    void app_pom(State& state, dcc::bidi::Datagram<>& datagram);
    void app_ext(State& state, dcc::bidi::Datagram<>& datagram);
    void app_info(State& state, dcc::bidi::Datagram<>& datagram);
    void app_dyn(State& state, dcc::bidi::Datagram<>& datagram);
    void app_xpom(State& state, dcc::bidi::Datagram<>& datagram);
    void app_cv_auto(State& state, dcc::bidi::Datagram<>& datagram);
    void app_block(State& state, dcc::bidi::Datagram<>& datagram);
    void app_tos(State& state, dcc::bidi::Datagram<>& datagram);
// clang-format on

//
void loco(State& state, dcc::bidi::Datagram<>& datagram) {
  channel1(state, datagram);
  channel2(state, datagram);
  if (std::ranges::all_of(datagram, [](auto b) { return !b; })) return;
  ImGui::SeparatorText("Done");
  if (ImGui::Button("Push to Datagrams"))
    state.loco_datagrams.push_back({.bytes = datagram});
}

//
void channel1(State& state, dcc::bidi::Datagram<>& datagram) {
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
void app_adr_high(State& state, dcc::bidi::Datagram<>& datagram) {}

//
void app_adr_low(State& state, dcc::bidi::Datagram<>& datagram) {}

//
void app_adr_info1(State& state, dcc::bidi::Datagram<>& datagram) {}

//
void channel2(State& state, dcc::bidi::Datagram<>& datagram) {
  ImGui::SeparatorText("Channel 2");
  static constexpr std::array ch2_datagrams{"",
                                            "app:pom",
                                            "app:ext",
                                            "app:info",
                                            "app:dyn",
                                            "app:xpom",
                                            "app:CV-auto",
                                            "app:block",
                                            "app:tos"};
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
  else if (!strcmp(ch2_datagrams[static_cast<size_t>(i)], "app:tos"))
    app_tos(state, datagram);
}

//
void app_pom(State& state, dcc::bidi::Datagram<>& datagram) {}

//
void app_ext(State& state, dcc::bidi::Datagram<>& datagram) {}

//
void app_info(State& state, dcc::bidi::Datagram<>& datagram) {}

//
void app_dyn(State& state, dcc::bidi::Datagram<>& datagram) {}

//
void app_xpom(State& state, dcc::bidi::Datagram<>& datagram) {}

//
void app_cv_auto(State& state, dcc::bidi::Datagram<>& datagram) {}

//
void app_block(State& state, dcc::bidi::Datagram<>& datagram) {}

//
void app_tos(State& state, dcc::bidi::Datagram<>& datagram) {}

} // namespace loco

namespace accessory {

//
void accessory(State& state, dcc::bidi::Datagram<>& datagram) {
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
      "Loco",
      "Accessory",
      // "Automatic Logon"
    };
    static int i{};
    ImGui::Combo(UNIQUE_LABEL(), &i, data(types), ssize(types));
    if (dcc::bidi::Datagram<> datagram{};
        !strcmp(types[static_cast<size_t>(i)], "Loco"))
      loco::loco(state, datagram);
    else if (!strcmp(types[static_cast<size_t>(i)], "Accessory"))
      accessory::accessory(state, datagram);
    else if (!strcmp(types[static_cast<size_t>(i)], "Automatic Logon"))
      ;
  }
  ImGui::End();
}
