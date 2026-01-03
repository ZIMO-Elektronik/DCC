#include "config.hpp"
#include <imgui.h>
#include <dcc/dcc.hpp>
#include "demo.hpp"
#include "query.hpp"
#include "state.hpp"

// Config window
void config() {
  if (!state.windows.show_config) return;

  if (ImGui::Begin("Config",
                   &state.windows.show_config,
                   ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::SeparatorText("Signal");
    uint8_t min{DCC_TX_MIN_PREAMBLE_BITS};
    uint8_t max{DCC_TX_MAX_PREAMBLE_BITS};
    ImGui::SliderScalar("Preamble Bits",
                        ImGuiDataType_U8,
                        &state.cfg.num_preamble,
                        &min,
                        &max,
                        "%u");

    min = dcc::tx::Timing::Bit1Min;
    max = dcc::tx::Timing::Bit1Max;
    ImGui::SliderScalar("Bit 1 Duration [us]",
                        ImGuiDataType_U8,
                        &state.cfg.bit1_duration,
                        &min,
                        &max,
                        "%u");

    min = dcc::tx::Timing::Bit0Min;
    max = dcc::tx::Timing::Bit0Max;
    ImGui::SliderScalar("Bit 0 Duration [us]",
                        ImGuiDataType_U8,
                        &state.cfg.bit0_duration,
                        &min,
                        &max,
                        "%u");

    min = 57u;
    max = 61u;
    ImGui::SliderScalar("Bit BiDi Duration [us]",
                        ImGuiDataType_U8,
                        &state.cfg.bidibit_duration,
                        &min,
                        &max,
                        "%u");

    ImGui::SeparatorText("How-To / Tips");
    ImGui::BulletText(
      "%s",
      "Use the Packet Builder to generate pre-defined or custom packets.");
    ImGui::BulletText(
      "%s", "Use the Datagram Builder to generate RailCom datagrams.");
    ImGui::BulletText("%s", "Hover over plots to highlight the current bit.");
    ImGui::BulletText("%s", "Use Ctrl+C and Ctrl+V to copy/paste a URL query.");
    ImGui::BulletText(
      "%s",
      "Share packets and datagrams by pasting a URL query into a browser.");
    ImGui::Bullet();
    ImGui::SameLine();
    if (ImGui::Button("tl;dr give me the demo!")) demo();
  }
  ImGui::End();
}
