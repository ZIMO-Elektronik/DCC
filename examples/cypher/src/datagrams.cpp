#include "datagrams.hpp"
#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>
#include "utility.hpp"

#define PRE_COL ImPlot::GetColormapColor(8)      // white
#define START_COL ImPlot::GetColormapColor(2)    // green
#define ADDR_COL ImPlot::GetColormapColor(5)     // yellow
#define DATA_COL ImPlot::GetColormapColor(4)     // orange
#define END_COL ImPlot::GetColormapColor(9)      // red
#define CHECKSUM_COL ImPlot::GetColormapColor(3) // violet

#define TCS_COL ImPlot::GetColormapColor(2)  // green
#define TTS1_COL ImPlot::GetColormapColor(5) // yellow
#define TTS2_COL ImPlot::GetColormapColor(4) // orange
#define TCE_COL ImPlot::GetColormapColor(9)  // red

namespace {

using namespace dcc::bidi;

namespace eval {

// clang-format off
void eval(State& state, State::Datagram& datagram);
  void tags(State::Datagram& datagram);
// clang-format on

void eval(State& state, State::Datagram& datagram) {
  if (!empty(datagram.desc_strs) && state.cfg == datagram.cfg) return;

  //
  datagram.cfg = state.cfg;

  // Clear/resize
  datagram.desc_strs.clear();
  datagram.pattern_str.clear();
  datagram.plots.t_p.clear();
  datagram.plots.p.clear();
  datagram.plots.t_b.clear();
  datagram.plots.b.clear();
  datagram.plots.highlights.clear();
  datagram.plots.tags.clear();

  // P
  for (auto i{0uz}; i <= 8uz; ++i) {
    auto const bit{static_cast<bool>(i % 2uz)};
    datagram.plots.t_p.push_back(i * datagram.cfg.bidibit_duration);
    datagram.plots.p.push_back(bit);
    datagram.plots.t_p.push_back(i * datagram.cfg.bidibit_duration);
    datagram.plots.p.push_back(!bit);
  }
  datagram.plots.t_p.push_back(Timing::TCEMax);
  datagram.plots.p.push_back(1.0);

  // BiDi
  static constexpr auto scale{0.5};
  datagram.plots.t_b.push_back(0.0);
  datagram.plots.b.push_back(1.0 * scale);
  for (auto i{0uz}; i < bundled_channels_size; ++i) {
    auto offset{i < 2uz ? Timing::TTS1
                        : Timing::TTS2 - channel1_size * 10u * 4u};
    offset += i * 10u * 4u;

    auto const b{datagram.bytes[i]};
    if (!b) break;

    // Startbit
    datagram.plots.t_b.push_back(offset + 0u * 4u);
    datagram.plots.b.push_back(1.0 * scale);
    datagram.plots.t_b.push_back(offset + 0u * 4u);
    datagram.plots.b.push_back(0.0);
    datagram.plots.t_b.push_back(offset + 1u * 4u);
    datagram.plots.b.push_back(0.0);

    bool bit;
    for (auto j{1uz}; j <= CHAR_BIT; ++j) {
      bit = static_cast<bool>(b & (1u << (j - 1u)));
      datagram.plots.t_b.push_back(offset + j * 4u);
      datagram.plots.b.push_back(bit * scale);
      datagram.plots.t_b.push_back(offset + (j + 1) * 4u);
      datagram.plots.b.push_back(bit * scale);
    }

    // Stopbit
    datagram.plots.t_b.push_back(offset + 9u * 4u);
    datagram.plots.b.push_back(bit * scale);
    datagram.plots.t_b.push_back(offset + 9u * 4u);
    datagram.plots.b.push_back(1.0 * scale);
    datagram.plots.t_b.push_back(offset + 10u * 4u);
    datagram.plots.b.push_back(1.0 * scale);
  }

  datagram.plots.t_b.push_back(datagram.plots.t_p.back());
  datagram.plots.b.push_back(1.0 * scale);

  datagram.desc_strs.push_back(" REMOVOOOVE ME ");

  tags(datagram);
}

//
void tags(State::Datagram& datagram) {
  datagram.plots.tags.push_back(
    {datagram.cfg.bidibit_duration / 2u, TCS_COL, "TCS"});
  datagram.plots.tags.push_back({Timing::TTS1, TTS1_COL, "TTS1"});
  datagram.plots.tags.push_back({Timing::TTS2, TTS2_COL, "TTS2"});
  datagram.plots.tags.push_back({Timing::TCEMax, TCE_COL, "TCE"});
}

} // namespace eval

////////////////////////////////////////////////////////////////////////////////

namespace tab {

// clang-format off
void tab(State::Datagram& datagram, size_t i);
  void description(State::Datagram& datagram);
  void data(State::Datagram& datagram);
  void plot(State::Datagram& datagram);
// clang-format on

//
void tab(State::Datagram& datagram, size_t i) {
  if (ImGui::BeginTabItem(("#" + std::to_string(i) + UNIQUE_LABEL()).c_str(),
                          &datagram.show,
                          ImGuiTabItemFlags_None)) {
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode(
          (std::string{"Description"} + UNIQUE_LABEL()).c_str())) {
      description(datagram);
      ImGui::TreePop();
    }
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode((std::string{"Data"} + UNIQUE_LABEL()).c_str())) {
      data(datagram);
      ImGui::TreePop();
    }
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode((std::string{"Plot"} + UNIQUE_LABEL()).c_str())) {
      plot(datagram);
      ImGui::TreePop();
    }
    ImGui::EndTabItem();
  }
}

//
void description(State::Datagram& datagram) {
  for (auto const& desc : datagram.desc_strs)
    ImGui::BulletText("%s", desc.c_str());
}

//
void data(State::Datagram& datagram) {
  auto const first{
    std::ranges::find_if(datagram.bytes, [](uint8_t b) { return b; })};
  auto const last{
    std::ranges::find_if(datagram.bytes, [](uint8_t b) { return !b; })};
  auto const offset{first - cbegin(datagram.bytes)};
  ImGui::BinaryTable(UNIQUE_LABEL(),
                     data(datagram.bytes) + offset,
                     last - first,
                     ImGuiInputTextFlags_ReadOnly,
                     offset);
}

//
void plot(State::Datagram& datagram) {
  if (ImPlot::BeginPlot("Digital Signal")) {
    // P track
    ImPlot::SetupAxis(ImAxis_X1, "Time [us]");
    ImPlot::SetupAxis(ImAxis_Y1, "P", ImPlotAxisFlags_NoTickLabels);
    ImPlot::SetupAxis(
      ImAxis_Y2, "N", ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_Opposite);
    ImPlot::SetupAxisLimits(ImAxis_X1,
                            -0.01 * datagram.plots.t_p.back(),
                            1.01 * datagram.plots.t_p.back());
    ImPlot::SetupAxisLimits(ImAxis_Y1, -4.0 + 1.0, 2.0);
    ImPlot::SetupAxisLimits(ImAxis_Y2, -2.0 + 1.0, 4.0);
    ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
    ImPlot::PlotLine("P",
                     data(datagram.plots.t_p),
                     data(datagram.plots.p),
                     static_cast<int>(ssize(datagram.plots.t_p)));
    ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
    ImPlot::PlotLine("BiDi",
                     data(datagram.plots.t_b),
                     data(datagram.plots.b),
                     static_cast<int>(ssize(datagram.plots.t_b)));

    // Tags
    for (auto const& [x, col, str] : datagram.plots.tags)
      ImPlot::TagX(x, col, "%s", str.c_str());

    // Highlight
    if (ImPlot::IsPlotHovered()) {
      ImPlotPoint const mouse{ImPlot::GetPlotMousePos()};
    }
  }

  ImPlot::EndPlot();
}

} // namespace tab

} // namespace

void datagrams(State& state) {
  if (!state.windows.show_datagrams) return;

  if (ImGui::Begin("Datagrams", &state.windows.show_datagrams)) {
    if (ImGui::BeginTabBar(UNIQUE_LABEL())) {
      size_t i{};

      for (auto& datagram : state.loco_datagrams) {
        eval::eval(state, datagram);
        tab::tab(datagram, i++);
      }

      for (auto& datagram : state.accessory_datagrams) {
        eval::eval(state, datagram);
        tab::tab(datagram, i++);
      }

      ImGui::EndTabBar();
    }
  }
  ImGui::End();
}
