#include "utility.hpp"
#include <array>
#include <bit>
#include <cassert>
#include <climits>
#include <cstdint>
#include <cstdlib>

namespace ImGui {

//
bool InputBinary(char const* label,
                 ImGuiDataType data_type,
                 void* p_data,
                 void const*,
                 void const*,
                 char const*,
                 ImGuiInputTextFlags flags) {
  assert(data_type == ImGuiDataType_U8);

  uint8_t& value{*static_cast<uint8_t*>(p_data)};
  std::array<char, 9uz> buf{};
  for (size_t i{CHAR_BIT}; i-- > 0uz;)
    buf[CHAR_BIT - 1uz - i] = ((value >> i) & 0b1u) ? '1' : '0';
  buf.back() = '\0';

  if (ImGui::InputText(label,
                       data(buf),
                       size(buf),
                       ImGuiInputTextFlags_CallbackCharFilter | flags,
                       [](ImGuiInputTextCallbackData* data) {
                         return static_cast<int>(
                           !(data->EventChar == '0' || data->EventChar == '1'));
                       })) {
    value = static_cast<uint8_t>(strtoul(data(buf), nullptr, 2));
    return true;
  }

  return false;
}

//
bool BinaryTable(char const* label,
                 uint8_t* p_data,
                 int rows,
                 ImGuiInputTextFlags flags) {
  if (ImGui::BeginTable(label, 4, ImGuiTableFlags_RowBg)) {
    ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Bin", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Dec", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Hex", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();
    for (int i{}; i < rows; ++i) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(std::to_string(i).c_str());
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-FLT_MIN);
      ImGui::InputBinary(UNIQUE_LABEL(i),
                         ImGuiDataType_U8,
                         p_data + i,
                         nullptr,
                         nullptr,
                         nullptr,
                         flags);
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-FLT_MIN);
      ImGui::InputScalar(UNIQUE_LABEL(i),
                         ImGuiDataType_U8,
                         p_data + i,
                         nullptr,
                         nullptr,
                         nullptr,
                         flags);
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-FLT_MIN);
      ImGui::InputScalar(UNIQUE_LABEL(i),
                         ImGuiDataType_U8,
                         p_data + i,
                         nullptr,
                         nullptr,
                         "%02X",
                         ImGuiInputTextFlags_CharsHexadecimal |
                           ImGuiInputTextFlags_CharsUppercase | flags);
    }
    ImGui::EndTable();
    return true;
  }
  return false;
}

} // namespace ImGui
