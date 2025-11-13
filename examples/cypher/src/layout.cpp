#include "layout.hpp"
#include <imgui.h>
#include <imgui_internal.h>

// ----------
// |aaaabbbb|
// |ccddbbbb|
// |ccddeeee|
// |ccddeeee|
// ----------
void layout() {
  ImGuiID dockspace_id{ImGui::GetID("MyDockSpace")};
  ImGuiViewport* viewport{ImGui::GetMainViewport()};

  if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr) {
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

    // Root dock: split horizontally for A+B
    ImGuiID dock_left{};
    ImGuiID dock_right{};
    ImGui::DockBuilderSplitNode(
      dockspace_id, ImGuiDir_Left, 0.5f, &dock_left, &dock_right);

    //
    ImGuiID dock_left_top{};
    ImGuiID dock_left_bottom{};
    ImGui::DockBuilderSplitNode(
      dock_left, ImGuiDir_Up, 0.25f, &dock_left_top, &dock_left_bottom);

    //
    ImGuiID dock_right_top{};
    ImGuiID dock_right_bottom{};
    ImGui::DockBuilderSplitNode(
      dock_right, ImGuiDir_Up, 0.5f, &dock_right_top, &dock_right_bottom);

    //
    ImGuiID dock_left_bottom_left{};
    ImGuiID dock_left_bottom_right{};
    ImGui::DockBuilderSplitNode(dock_left_bottom,
                                ImGuiDir_Left,
                                0.5f,
                                &dock_left_bottom_left,
                                &dock_left_bottom_right);

    // Dock your windows
    ImGui::DockBuilderDockWindow("Config", dock_left_top);
    ImGui::DockBuilderDockWindow("Packet Builder", dock_left_bottom_left);
    ImGui::DockBuilderDockWindow("Datagram Builder", dock_left_bottom_right);
    ImGui::DockBuilderDockWindow("Packets", dock_right_top);
    ImGui::DockBuilderDockWindow("Datagrams", dock_right_bottom);

    ImGui::DockBuilderFinish(dockspace_id);
  }

  ImGui::DockSpaceOverViewport(
    dockspace_id, viewport, ImGuiDockNodeFlags_PassthruCentralNode);
}
