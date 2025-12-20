#include "datagrams.hpp"
#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>
#include "utility.hpp"

#define START_COL ImPlot::GetColormapColor(2) // green
#define STOP_COL ImPlot::GetColormapColor(9)  // red
#define TCS_COL ImPlot::GetColormapColor(8)   // white
#define TTS1_COL ImPlot::GetColormapColor(5)  // yellow
#define TTC1_COL TTS1_COL
#define TTS2_COL ImPlot::GetColormapColor(4) // orange
#define TTC2_COL TTS2_COL
#define TCE_COL ImPlot::GetColormapColor(3) // violet

namespace {

using namespace dcc::bidi;

namespace eval {

// clang-format off
void eval(State& state, State::Datagram& datagram);
  void dissector(State::Datagram& datagram);
  void highlights(State::Datagram& datagram);
  void tags(State::Datagram& datagram);
// clang-format on

} // namespace eval

namespace tab {

// clang-format off
void tab(State::Datagram& datagram, size_t i);
  void description(State::Datagram& datagram);
  void data(State::Datagram& datagram);
  void plot(State::Datagram& datagram);
// clang-format on

} // namespace tab

namespace eval {

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

  // BiDi
  static constexpr auto scale{0.5};
  datagram.plots.t_b.push_back(0.0);
  datagram.plots.b.push_back(1.0 * scale);
  for (auto i{0uz}; i < bundled_channels_size; ++i) {
    auto offset{i < 2uz ? Timing::TTS1
                        : Timing::TTS2 - channel1_size * 10u * 4u};
    offset += i * 10u * 4u;

    auto const b{datagram.bytes[i]};
    if (!b) continue;

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

  dissector(datagram);
  highlights(datagram);
  tags(datagram);
}

//
void dissector(State::Datagram& datagram) {
  Dissector dissector{datagram.bytes, datagram.addr};
  for (auto const& dg : dissector) {
    if (auto const ack{get_if<Ack>(&dg)}) {
      datagram.desc_strs.push_back("ACK");
    } else if (auto const nak{get_if<Nak>(&dg)}) {
      datagram.desc_strs.push_back("NAK");
    } else if (auto const adr_low{get_if<app::Pom>(&dg)}) {
      datagram.desc_strs.push_back("app:pom");
    } else if (auto const dyn{get_if<app::AdrHigh>(&dg)}) {
      datagram.desc_strs.push_back("app:adr_high");
    } else if (auto const dyn{get_if<app::AdrLow>(&dg)}) {
      datagram.desc_strs.push_back("app:adr_low");
    } else if (auto const dyn{get_if<app::Ext>(&dg)}) {
      datagram.desc_strs.push_back("app:ext");
    } else if (auto const dyn{get_if<app::Info>(&dg)}) {
      datagram.desc_strs.push_back("app:info");
    } else if (auto const dyn{get_if<app::Dyn>(&dg)}) {
      datagram.desc_strs.push_back("app:dyn");
    } else if (auto const dyn{get_if<app::Xpom>(&dg)}) {
      datagram.desc_strs.push_back("app:xpom");
    } else if (auto const dyn{get_if<app::CvAuto>(&dg)}) {
      datagram.desc_strs.push_back("app:CV-auto");
    } else if (auto const dyn{get_if<app::Block>(&dg)}) {
      datagram.desc_strs.push_back("app:block");
    } else if (auto const dyn{get_if<app::Search>(&dg)}) {
      datagram.desc_strs.push_back("app:search");
    } else if (auto const dyn{get_if<app::Srq>(&dg)}) {
      datagram.desc_strs.push_back("app:srq");
    } else if (auto const dyn{get_if<app::Stat4>(&dg)}) {
      datagram.desc_strs.push_back("app:stat4");
    } else if (auto const dyn{get_if<app::Stat1>(&dg)}) {
      datagram.desc_strs.push_back("app:stat1");
    } else if (auto const dyn{get_if<app::Time>(&dg)}) {
      datagram.desc_strs.push_back("app:time");
    } else if (auto const dyn{get_if<app::Error>(&dg)}) {
      datagram.desc_strs.push_back("app:error");
    } else if (auto const dyn{get_if<app::Test>(&dg)}) {
      datagram.desc_strs.push_back("app:test");
    }
  }
}

//
void highlights(State::Datagram& datagram) {
  // Build data string
  std::string data_str{};
  for (auto const b : datagram.bytes)
    if (b) {
      auto str{std::format(" {:08b} ", b)};
      std::ranges::reverse(str); // LSB first
      data_str += str;
    }

  auto const hgl_str{data_str + "\n"};

  // Carot index
  size_t c{};

  for (auto i{0uz}; i < bundled_channels_size; ++i) {
    auto offset{i < 2uz ? Timing::TTS1
                        : Timing::TTS2 - channel1_size * 10u * 4u};
    offset += i * 10u * 4u;

    auto const b{datagram.bytes[i]};
    if (!b) continue;

    // Startbit
    datagram.plots.highlights.push_back(
      {START_COL,
       hgl_str + std::string(size(hgl_str), ' ').replace(c, 1uz, 1uz, '^')});
    datagram.plots.highlights.push_back(
      {START_COL,
       hgl_str + std::string(size(hgl_str), ' ').replace(c, 1uz, 1uz, '^')});
    datagram.plots.highlights.push_back(
      {START_COL,
       hgl_str + std::string(size(hgl_str), ' ').replace(c++, 1uz, 1uz, '^')});

    auto const col{i < 2uz ? TTS1_COL : TTS2_COL};
    for (auto j{1uz}; j <= CHAR_BIT; ++j) {
      datagram.plots.highlights.push_back(
        {col,
         hgl_str + std::string(size(hgl_str), ' ').replace(c, 1uz, 1uz, '^')});
      datagram.plots.highlights.push_back(
        {col,
         hgl_str +
           std::string(size(hgl_str), ' ').replace(c++, 1uz, 1uz, '^')});
    }

    // Stopbit
    datagram.plots.highlights.push_back(
      {STOP_COL,
       hgl_str + std::string(size(hgl_str), ' ').replace(c, 1uz, 1uz, '^')});
    datagram.plots.highlights.push_back(
      {STOP_COL,
       hgl_str + std::string(size(hgl_str), ' ').replace(c, 1uz, 1uz, '^')});
    datagram.plots.highlights.push_back(
      {STOP_COL,
       hgl_str + std::string(size(hgl_str), ' ').replace(c++, 1uz, 1uz, '^')});
  }
}

//
void tags(State::Datagram& datagram) {
  datagram.plots.tags.push_back(
    {datagram.cfg.bidibit_duration / 2u, TCS_COL, "TCS"});
  datagram.plots.tags.push_back({Timing::TTS1, TTS1_COL, "TTS1"});
  datagram.plots.tags.push_back({Timing::TTC1, TTC1_COL, "TTC1"});
  datagram.plots.tags.push_back({Timing::TTS2, TTS2_COL, "TTS2"});
  datagram.plots.tags.push_back({Timing::TTC2, TTC1_COL, "TTC2"});
  datagram.plots.tags.push_back(
    {8 * datagram.cfg.bidibit_duration, TCE_COL, "TCE"});
}

} // namespace eval

////////////////////////////////////////////////////////////////////////////////

namespace tab {

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
  // This is beyond stupid, but have you tried doing that shit with algorithms?
  int first{-1};
  int last{-1};
  for (auto i{0}; i < ssize(datagram.bytes); ++i) {
    if (!datagram.bytes[i]) continue;
    if (first == -1) first = i;
    last = i;
  }
  ImGui::BinaryTable(UNIQUE_LABEL(),
                     data(datagram.bytes) + first,
                     last - first + 1,
                     ImGuiInputTextFlags_ReadOnly,
                     first);
}

//
void plot(State::Datagram& datagram) {
  if (ImPlot::BeginPlot("Digital Signal")) {
    // Plot P and BiDi
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

    // Highlights
    if (ImPlot::IsPlotHovered()) {
      ImPlotPoint const mouse{ImPlot::GetPlotMousePos()};

      uint8_t b{};
      //
      if (mouse.x >= Timing::TTS1 &&
          mouse.x <= Timing::TTS1 + 10u * 4u * channel1_size)
        b =
          datagram
            .bytes[static_cast<size_t>((mouse.x - Timing::TTS1) / (10u * 4u))];
      //
      else if (mouse.x >= Timing::TTS2 &&
               mouse.x <= Timing::TTS2 + 10u * 4u * channel2_size)
        b = datagram
              .bytes[channel1_size + static_cast<size_t>(
                                       (mouse.x - Timing::TTS2) / (10u * 4u))];

      if (auto const it{std::ranges::adjacent_find(
            datagram.plots.t_b,
            [x = mouse.x](double a, double b) { return x >= a && x <= b; })};
          b && it != cend(datagram.plots.t_b)) {
        auto const i{static_cast<size_t>(
          std::ranges::distance(cbegin(datagram.plots.t_b), it))};

        // Convert segment x coords to pixel space
        std::array const ps{
          ImPlot::PlotToPixels(ImPlotPoint{datagram.plots.t_b[i], 0}),
          ImPlot::PlotToPixels(ImPlotPoint{datagram.plots.t_b[i + 1uz], 0})};

        // Get plot rect (for full-height fill)
        ImVec2 const plot_pos{ImPlot::GetPlotPos()};
        ImVec2 const plot_size{ImPlot::GetPlotSize()};
        float const y_min{plot_pos.y};
        float const y_max{plot_pos.y + plot_size.y};

        ImDrawList* dl{ImPlot::GetPlotDrawList()};

        // Filled translucent rectangle covering the signal region
        auto const& [hgl_col, hgl_str]{datagram.plots.highlights[i]};
        dl->AddRectFilled(
          ImVec2{ps[0uz].x, y_min},
          ImVec2{ps[1uz].x, y_max},
          ImColor(
            hgl_col.Value.x, hgl_col.Value.y, hgl_col.Value.z, 50 / 255.0f));

        // Optional vertical line at the exact mouse x
        ImVec2 const px{ImPlot::PlotToPixels(ImPlotPoint{mouse.x, 0.0})};
        dl->AddLine(
          ImVec2{px.x, y_min},
          ImVec2{px.x, y_max},
          ImColor(
            hgl_col.Value.x, hgl_col.Value.y, hgl_col.Value.z, 150 / 255.0f),
          1.0f);

        ImGui::BeginTooltip();
        ImGui::TextUnformatted(hgl_str.c_str());
        ImGui::EndTooltip();
      }
    }

    // Tags
    for (auto const& [x, col, str] : datagram.plots.tags)
      ImPlot::TagX(x, col, "%s", str.c_str());

    ImPlot::EndPlot();
  }
}

} // namespace tab

} // namespace

void datagrams(State& state) {
  if (!state.windows.show_datagrams) return;

  if (ImGui::Begin("Datagrams", &state.windows.show_datagrams)) {
    if (ImGui::BeginTabBar(UNIQUE_LABEL())) {
      size_t i{};

      for (auto& datagram : state.datagrams) {
        eval::eval(state, datagram);
        tab::tab(datagram, i++);
      }

      ImGui::EndTabBar();
    }
  }
  ImGui::End();
}
