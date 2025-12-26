#include "config.hpp"
#include <imgui.h>
#include <dcc/dcc.hpp>
#include "query.hpp"

// Config window
void config(State& state) {
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

    ImGui::SeparatorText("Share");
    if (ImGui::Button("To Clipboard")) {
      ImGui::SetClipboardText("");
      ImGui::LogToClipboard();
      ImGui::LogText("%s", to_query(state).c_str());
      ImGui::LogFinish();
    }
#if !defined(__EMSCRIPTEN__)
    if (ImGui::Button("From Clipboard"))
      from_query(state, ImGui::GetClipboardText());
#endif
  }
  ImGui::End();
}
