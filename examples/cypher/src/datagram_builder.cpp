#include "datagram_builder.hpp"
#include <imgui.h>

void datagram_builder(State& state) {
  if (!state.windows.show_datagram_builder) return;

  if (ImGui::Begin("Datagram Builder",
                   &state.windows.show_datagram_builder,
                   ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("\\todo");
  }
  ImGui::End();
}
