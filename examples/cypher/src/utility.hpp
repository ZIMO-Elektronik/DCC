#pragma once

#include <imgui.h>
#include <array>
#include <concepts>
#include <cstdint>
#include <dcc/dcc.hpp>
#include <random>
#include <string>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// Create a unique label to pass to ImGui::*
//
// \warning
// The lifetime of the std::string only sufficient if used directly as in
// ```
// ImGui::Button(IMGUI_UNIQUE_ID(42));
// ```
//
// This would be UB
// ```
// const char* id{IMGUI_UNIQUE_ID(42)};
// ImGui::Button(id);
// ```
#define UNIQUE_LABEL(...)                                                      \
  (sizeof(#__VA_ARGS__) > 1uz                                                  \
     ? (std::string("##") + __FILE__ + TOSTRING(__LINE__) +                    \
        std::to_string(int(__VA_ARGS__)))                                      \
         .c_str()                                                              \
     : ("##" __FILE__ TOSTRING(__LINE__)))

//
inline constexpr std::array speed_labels{
  "EStop", "Stop", "1",   "2",   "3",   "4",   "5",   "6",   "7",   "8",
  "9",     "10",   "11",  "12",  "13",  "14",  "15",  "16",  "17",  "18",
  "19",    "20",   "21",  "22",  "23",  "24",  "25",  "26",  "27",  "28",
  "29",    "30",   "31",  "32",  "33",  "34",  "35",  "36",  "37",  "38",
  "39",    "40",   "41",  "42",  "43",  "44",  "45",  "46",  "47",  "48",
  "49",    "50",   "51",  "52",  "53",  "54",  "55",  "56",  "57",  "58",
  "59",    "60",   "61",  "62",  "63",  "64",  "65",  "66",  "67",  "68",
  "69",    "70",   "71",  "72",  "73",  "74",  "75",  "76",  "77",  "78",
  "79",    "80",   "81",  "82",  "83",  "84",  "85",  "86",  "87",  "88",
  "89",    "90",   "91",  "92",  "93",  "94",  "95",  "96",  "97",  "98",
  "99",    "100",  "101", "102", "103", "104", "105", "106", "107", "108",
  "109",   "110",  "111", "112", "113", "114", "115", "116", "117", "118",
  "119",   "120",  "121", "122", "123", "124", "125", "126"};

//
template<std::unsigned_integral T>
constexpr T random_interval(T min, T max) {
  std::mt19937 gen{std::random_device{}()};
  std::uniform_int_distribution<T> dis{min, max};
  return dis(gen);
}

//
constexpr auto random_basic_loco() {
  return dcc::Address{.value = random_interval<uint16_t>(1u, 127u),
                      .type = dcc::Address::BasicLoco};
}

//
constexpr auto random_extended_loco() {
  return dcc::Address{.value = random_interval<uint16_t>(1u, 10239u),
                      .type = dcc::Address::ExtendedLoco};
}

//
constexpr auto random_basic_accessory() {
  return dcc::Address{.value = random_interval<uint16_t>(0u, 2047u),
                      .type = dcc::Address::BasicAccessory};
}

//
constexpr auto random_extended_accessory() {
  return dcc::Address{.value = random_interval<uint16_t>(0u, 2047u),
                      .type = dcc::Address::ExtendedAccessory};
}

namespace ImGui {

bool InputBinary(char const* label,
                 ImGuiDataType data_type,
                 void* p_data,
                 void const* p_step = nullptr,
                 void const* p_step_fast = nullptr,
                 char const* format = nullptr,
                 ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);
bool BinaryTable(char const* label,
                 uint8_t* p_data,
                 int rows,
                 ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);

} // namespace ImGui
