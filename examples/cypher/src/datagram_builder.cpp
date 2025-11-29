#include "datagram_builder.hpp"
#include <imgui.h>
#include "utility.hpp"

void datagram_builder(State& state) {
  if (!state.windows.show_datagram_builder) return;

  if (ImGui::Begin("Datagram Builder",
                   &state.windows.show_datagram_builder,
                   ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::SeparatorText("Type");
    static constexpr std::array types{"", "Loco", "Accessory"};
    static int i{};
    ImGui::Combo(UNIQUE_LABEL(), &i, data(types), ssize(types));
  }
  ImGui::End();
}
