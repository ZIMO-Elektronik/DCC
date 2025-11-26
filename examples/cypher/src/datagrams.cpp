#include "datagrams.hpp"
#include <imgui.h>
#include "query.hpp"

void datagrams(State& state) {
  if (!state.windows.show_datagrams) return;

  if (ImGui::Begin("Datagrams", &state.windows.show_datagrams)) {
    ImGui::Text("\\todo");

    if (ImGui::Button("Copy \"Hello, world!\" to clipboard")) {
      ImGui::LogToClipboard();
      auto STR = build_query(state);
      ImGui::LogText("%s", STR.c_str());
      ImGui::LogFinish();
    }
  }
  ImGui::End();
}
