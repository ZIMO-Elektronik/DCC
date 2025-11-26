#include "config.hpp"
#include <imgui.h>
#include <dcc/dcc.hpp>
#include "query.hpp"

void config(State& state) {
  if (!state.windows.show_config) return;

  if (ImGui::Begin("Config",
                   &state.windows.show_config,
                   ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::SeparatorText("Signal");
    uint8_t min{DCC_TX_MIN_PREAMBLE_BITS};
    uint8_t max{DCC_TX_MAX_PREAMBLE_BITS};
    ImGui::SliderScalar("Preamble bits",
                        ImGuiDataType_U8,
                        &state.cfg.num_preamble,
                        &min,
                        &max,
                        "%u");

    min = dcc::tx::Timing::Bit1Min;
    max = dcc::tx::Timing::Bit1Max;
    ImGui::SliderScalar("Bit 1 duration [us]",
                        ImGuiDataType_U8,
                        &state.cfg.bit1_duration,
                        &min,
                        &max,
                        "%u");

    min = dcc::tx::Timing::Bit0Min;
    max = dcc::tx::Timing::Bit0Max;
    ImGui::SliderScalar("Bit 0 duration [us]",
                        ImGuiDataType_U8,
                        &state.cfg.bit0_duration,
                        &min,
                        &max,
                        "%u");

    ImGui::SeparatorText("Share");
    if (ImGui::Button("To clipboard")) {
      ImGui::LogToClipboard();
      ImGui::LogText("%s", to_query(state).c_str());
      ImGui::LogFinish();
    }
    if (ImGui::Button("From clipboard"))
      from_query(state, ImGui::GetClipboardText());
  }
  ImGui::End();
}
