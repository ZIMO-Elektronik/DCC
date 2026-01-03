#include "menu.hpp"
#include <imgui.h>
#include "state.hpp"

// Menu window
void menu() {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("View")) {
      ImGui::MenuItem("Config", nullptr, &state.windows.show_config);
      ImGui::MenuItem(
        "Packet Builder", nullptr, &state.windows.show_packet_builder);
      ImGui::MenuItem(
        "Datagram Builder", nullptr, &state.windows.show_datagram_builder);
      ImGui::MenuItem("Packets", nullptr, &state.windows.show_packets);
      ImGui::MenuItem("Datagrams", nullptr, &state.windows.show_datagrams);
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help")) {
      ImGui::MenuItem("About", nullptr, &state.windows.show_about);
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
}
