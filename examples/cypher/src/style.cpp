#include "style.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

void style() {
  // #B2D2F0
  static constexpr ImColor light_blue(0xB2, 0xD2, 0xF0, 0xFF);

  // #365F91
  static constexpr ImColor dark_blue(0x36, 0x5F, 0x91, 0xFF);

  // #FFFFFE
  static constexpr ImColor yellow(0xFF, 0xFF, 0xFE, 0xFF);

  // ImGui
  {
    ImGuiStyle* style{&ImGui::GetStyle()};
    ImVec4* colors{style->Colors};

    // clang-format off
    colors[ImGuiCol_Text]                      = ImVec4{1.00f, 1.00f, 1.00f, 1.00f};
    colors[ImGuiCol_TextDisabled]              = ImVec4{0.50f, 0.50f, 0.50f, 1.00f};
    colors[ImGuiCol_WindowBg]                  = ImVec4{0.06f, 0.06f, 0.06f, 0.94f};
    colors[ImGuiCol_ChildBg]                   = ImVec4{0.00f, 0.00f, 0.00f, 0.00f};
    colors[ImGuiCol_PopupBg]                   = ImVec4{0.08f, 0.08f, 0.08f, 0.94f};
    colors[ImGuiCol_Border]                    = ImVec4{0.43f, 0.43f, 0.50f, 0.50f};
    colors[ImGuiCol_BorderShadow]              = ImVec4{0.00f, 0.00f, 0.00f, 0.00f};
    colors[ImGuiCol_FrameBg]                   = ImVec4{dark_blue.Value.x, dark_blue.Value.y, dark_blue.Value.z, 0.54f};
    colors[ImGuiCol_FrameBgHovered]            = ImVec4{light_blue.Value.x, light_blue.Value.y, light_blue.Value.z, 0.40f};
    colors[ImGuiCol_FrameBgActive]             = ImVec4{light_blue.Value.x, light_blue.Value.y, light_blue.Value.z, 0.67f};
    colors[ImGuiCol_TitleBg]                   = ImVec4{0.04f, 0.04f, 0.04f, 1.00f};
    colors[ImGuiCol_TitleBgActive]             = ImVec4{dark_blue.Value.x, dark_blue.Value.y, dark_blue.Value.z, 1.00f};
    colors[ImGuiCol_TitleBgCollapsed]          = ImVec4{0.00f, 0.00f, 0.00f, 0.51f};
    colors[ImGuiCol_MenuBarBg]                 = ImVec4{0.14f, 0.14f, 0.14f, 1.00f};
    colors[ImGuiCol_ScrollbarBg]               = ImVec4{0.02f, 0.02f, 0.02f, 0.53f};
    colors[ImGuiCol_ScrollbarGrab]             = ImVec4{0.31f, 0.31f, 0.31f, 1.00f};
    colors[ImGuiCol_ScrollbarGrabHovered]      = ImVec4{0.41f, 0.41f, 0.41f, 1.00f};
    colors[ImGuiCol_ScrollbarGrabActive]       = ImVec4{0.51f, 0.51f, 0.51f, 1.00f};
    colors[ImGuiCol_CheckMark]                 = ImVec4{light_blue.Value.x, light_blue.Value.y, light_blue.Value.z, 1.00f};
    colors[ImGuiCol_SliderGrab]                = ImVec4{dark_blue.Value.x, dark_blue.Value.y, dark_blue.Value.z, 1.00f};
    colors[ImGuiCol_SliderGrabActive]          = ImVec4{light_blue.Value.x, light_blue.Value.y, light_blue.Value.z, 1.00f};
    colors[ImGuiCol_Button]                    = ImVec4{dark_blue.Value.x, dark_blue.Value.y, dark_blue.Value.z, 0.40f};
    colors[ImGuiCol_ButtonHovered]             = ImVec4{dark_blue.Value.x, dark_blue.Value.y, dark_blue.Value.z, 1.00f};
    colors[ImGuiCol_ButtonActive]              = ImVec4{light_blue.Value.x, light_blue.Value.y, light_blue.Value.z, 1.00f};
    colors[ImGuiCol_Header]                    = ImVec4{light_blue.Value.x, light_blue.Value.y, light_blue.Value.z, 0.31f};
    colors[ImGuiCol_HeaderHovered]             = ImVec4{light_blue.Value.x, light_blue.Value.y, light_blue.Value.z, 0.80f};
    colors[ImGuiCol_HeaderActive]              = ImVec4{light_blue.Value.x, light_blue.Value.y, light_blue.Value.z, 1.00f};
    colors[ImGuiCol_Separator]                 = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered]          = ImVec4{0.10f, 0.40f, 0.75f, 0.78f};
    colors[ImGuiCol_SeparatorActive]           = ImVec4{0.10f, 0.40f, 0.75f, 1.00f};
    colors[ImGuiCol_ResizeGrip]                = ImVec4{light_blue.Value.x, light_blue.Value.y, light_blue.Value.z, 0.20f};
    colors[ImGuiCol_ResizeGripHovered]         = ImVec4{light_blue.Value.x, light_blue.Value.y, light_blue.Value.z, 0.67f};
    colors[ImGuiCol_ResizeGripActive]          = ImVec4{light_blue.Value.x, light_blue.Value.y, light_blue.Value.z, 0.95f};
    colors[ImGuiCol_TabHovered]                = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_Tab]                       = ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.80f);
    colors[ImGuiCol_TabSelected]               = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabSelectedOverline]       = colors[ImGuiCol_HeaderActive];
    colors[ImGuiCol_TabDimmed]                 = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabDimmedSelected]         = ImLerp(colors[ImGuiCol_TabSelected],  colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4{0.50f, 0.50f, 0.50f, 0.00f};
    colors[ImGuiCol_DockingPreview]            = colors[ImGuiCol_HeaderActive] * ImVec4{1.0f, 1.0f, 1.0f, 0.7f};
    colors[ImGuiCol_DockingEmptyBg]            = ImVec4{0.20f, 0.20f, 0.20f, 1.00f};
    colors[ImGuiCol_PlotLines]                 = ImVec4{0.61f, 0.61f, 0.61f, 1.00f};
    colors[ImGuiCol_PlotLinesHovered]          = ImVec4{1.00f, 0.43f, 0.35f, 1.00f};
    colors[ImGuiCol_PlotHistogram]             = ImVec4{0.90f, 0.70f, 0.00f, 1.00f};
    colors[ImGuiCol_PlotHistogramHovered]      = ImVec4{1.00f, 0.60f, 0.00f, 1.00f};
    colors[ImGuiCol_TableHeaderBg]             = ImVec4{0.19f, 0.19f, 0.20f, 1.00f};
    colors[ImGuiCol_TableBorderStrong]         = ImVec4{0.31f, 0.31f, 0.35f, 1.00f};
    colors[ImGuiCol_TableBorderLight]          = ImVec4{0.23f, 0.23f, 0.25f, 1.00f};
    colors[ImGuiCol_TableRowBg]                = ImVec4{0.00f, 0.00f, 0.00f, 0.00f};
    colors[ImGuiCol_TableRowBgAlt]             = ImVec4{1.00f, 1.00f, 1.00f, 0.06f};
    colors[ImGuiCol_TextLink]                  = colors[ImGuiCol_HeaderActive];
    colors[ImGuiCol_TextSelectedBg]            = ImVec4{light_blue.Value.x, light_blue.Value.y, light_blue.Value.z, 0.35f};
    colors[ImGuiCol_DragDropTarget]            = ImVec4{1.00f, 1.00f, 0.00f, 0.90f};
    colors[ImGuiCol_NavCursor]                 = ImVec4{light_blue.Value.x, light_blue.Value.y, light_blue.Value.z, 1.00f};
    colors[ImGuiCol_NavWindowingHighlight]     = ImVec4{1.00f, 1.00f, 1.00f, 0.70f};
    colors[ImGuiCol_NavWindowingDimBg]         = ImVec4{0.80f, 0.80f, 0.80f, 0.20f};
    colors[ImGuiCol_ModalWindowDimBg]          = ImVec4{0.80f, 0.80f, 0.80f, 0.35f};
    // clang-format on
  }

  // ImPlot
  {
    ImPlotStyle* style{&ImPlot::GetStyle()};
    ImVec4* colors{style->Colors};

    // clang-format off
    style->MinorAlpha                          = 0.25f;
    style->Colormap                            = ImPlotColormap_Dark;
    style->Use24HourClock                      = true;
    colors[ImPlotCol_Line]                     = IMPLOT_AUTO_COL;
    colors[ImPlotCol_Fill]                     = IMPLOT_AUTO_COL;
    colors[ImPlotCol_MarkerOutline]            = IMPLOT_AUTO_COL;
    colors[ImPlotCol_MarkerFill]               = IMPLOT_AUTO_COL;
    colors[ImPlotCol_ErrorBar]                 = IMPLOT_AUTO_COL;
    colors[ImPlotCol_FrameBg]                  = ImVec4{1.00f, 1.00f, 1.00f, 0.07f};
    colors[ImPlotCol_PlotBg]                   = ImVec4{0.00f, 0.00f, 0.00f, 0.50f};
    colors[ImPlotCol_PlotBorder]               = ImVec4{0.43f, 0.43f, 0.50f, 0.50f};
    colors[ImPlotCol_LegendBg]                 = ImVec4{0.08f, 0.08f, 0.08f, 0.94f};
    colors[ImPlotCol_LegendBorder]             = ImVec4{0.43f, 0.43f, 0.50f, 0.50f};
    colors[ImPlotCol_LegendText]               = ImVec4{1.00f, 1.00f, 1.00f, 1.00f};
    colors[ImPlotCol_TitleText]                = ImVec4{1.00f, 1.00f, 1.00f, 1.00f};
    colors[ImPlotCol_InlayText]                = ImVec4{1.00f, 1.00f, 1.00f, 1.00f};
    colors[ImPlotCol_AxisText]                 = ImVec4{1.00f, 1.00f, 1.00f, 1.00f};
    colors[ImPlotCol_AxisGrid]                 = ImVec4{1.00f, 1.00f, 1.00f, 0.25f};
    colors[ImPlotCol_AxisTick]                 = IMPLOT_AUTO_COL; // TODO
    colors[ImPlotCol_AxisBg]                   = IMPLOT_AUTO_COL; // TODO
    colors[ImPlotCol_AxisBgHovered]            = IMPLOT_AUTO_COL; // TODO
    colors[ImPlotCol_AxisBgActive]             = IMPLOT_AUTO_COL; // TODO
    colors[ImPlotCol_Selection]                = ImVec4{1.00f, 0.60f, 0.00f, 1.00f};
    colors[ImPlotCol_Crosshairs]               = ImVec4{1.00f, 1.00f, 1.00f, 0.50f};
    // clang-format on
  }
}
