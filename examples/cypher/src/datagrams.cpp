#include "datagrams.hpp"
#include <imgui.h>

void datagrams(State& state) {
  if (!state.windows.show_datagrams) return;

  if (ImGui::Begin("Datagrams", &state.windows.show_datagrams)) {
    ImGui::Text("\\todo");

    if (ImGui::Button("Copy \"Hello, world!\" to clipboard")) {
      ImGui::LogToClipboard();
      ImGui::LogText("Hello, world!");
      ImGui::LogFinish();
    }
  }
  ImGui::End();
}
