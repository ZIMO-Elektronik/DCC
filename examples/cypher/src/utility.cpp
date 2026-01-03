#include "utility.hpp"
#include <array>
#include <bit>
#include <cassert>
#include <climits>
#include <cstdint>
#include <cstdlib>

// Random basic or extended loco address
dcc::Address random_loco_address() {
  return random_interval(0, 1) ? random_basic_loco_address()
                               : random_extended_loco_address();
}

// Random basic loco address
dcc::Address random_basic_loco_address() {
  return dcc::Address{.value = random_interval<uint16_t>(1u, 127u),
                      .type = dcc::Address::BasicLoco};
}

// Random extended loco address
dcc::Address random_extended_loco_address() {
  return dcc::Address{.value = random_interval<uint16_t>(1u, 10239u),
                      .type = dcc::Address::ExtendedLoco};
}

// Random basic accessory address
dcc::Address random_basic_accessory_address() {
  return dcc::Address{.value = random_interval<uint16_t>(0u, 2047u),
                      .type = dcc::Address::BasicAccessory};
}

// Random extended accessory address
dcc::Address random_extended_accessory_address() {
  return dcc::Address{.value = random_interval<uint16_t>(0u, 2047u),
                      .type = dcc::Address::ExtendedAccessory};
}

// Check if address is MOB
bool is_mob_address(dcc::Address addr) {
  return addr.type == dcc::Address::Broadcast ||
         addr.type == dcc::Address::BasicLoco ||
         addr.type == dcc::Address::ExtendedLoco;
}

namespace ImGui {

// Like InputScalar, but with binary
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

// Table which shows binary, decimal and hex values
bool BinaryTable(char const* label,
                 uint8_t* p_data,
                 int rows,
                 ImGuiInputTextFlags flags,
                 int numbering_offset) {
  if (ImGui::BeginTable(label, 4, ImGuiTableFlags_RowBg)) {
    ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Bin", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Dec", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Hex", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();
    for (int i{}; i < rows; ++i) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(std::to_string(numbering_offset + i).c_str());
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
