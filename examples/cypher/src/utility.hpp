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
inline constexpr std::array weekday_labels{"Monday",
                                           "Tuesday",
                                           "Wednesday",
                                           "Thursday",
                                           "Friday",
                                           "Saturday",
                                           "Sunday",
                                           "Not Supported"};

//
inline constexpr std::array switch_on_time_labels{
  "Off",  "0.1",  "0.2",  "0.3",  "0.4",  "0.5",  "0.6",  "0.7",  "0.8",
  "0.9",  "1.0",  "1.1",  "1.2",  "1.3",  "1.4",  "1.5",  "1.6",  "1.7",
  "1.8",  "1.9",  "2.0",  "2.1",  "2.2",  "2.3",  "2.4",  "2.5",  "2.6",
  "2.7",  "2.8",  "2.9",  "3.0",  "3.1",  "3.2",  "3.3",  "3.4",  "3.5",
  "3.6",  "3.7",  "3.8",  "3.9",  "4.0",  "4.1",  "4.2",  "4.3",  "4.4",
  "4.5",  "4.6",  "4.7",  "4.8",  "4.9",  "5.0",  "5.1",  "5.2",  "5.3",
  "5.4",  "5.5",  "5.6",  "5.7",  "5.8",  "5.9",  "6.0",  "6.1",  "6.2",
  "6.3",  "6.4",  "6.5",  "6.6",  "6.7",  "6.8",  "6.9",  "7.0",  "7.1",
  "7.2",  "7.3",  "7.4",  "7.5",  "7.6",  "7.7",  "7.8",  "7.9",  "8.0",
  "8.1",  "8.2",  "8.3",  "8.4",  "8.5",  "8.6",  "8.7",  "8.8",  "8.9",
  "9.0",  "9.1",  "9.2",  "9.3",  "9.4",  "9.5",  "9.6",  "9.7",  "9.8",
  "9.9",  "10.0", "10.1", "10.2", "10.3", "10.4", "10.5", "10.6", "10.7",
  "10.8", "10.9", "11.0", "11.1", "11.2", "11.3", "11.4", "11.5", "11.6",
  "11.7", "11.8", "11.9", "12.0", "12.1", "12.2", "12.3", "12.4", "12.5",
  "12.6", "On"};

//
inline constexpr std::array analog_labels{"0 (Reserved)",
                                          "1 (Volume)",
                                          "2 (Reserved)",
                                          "3 (Reserved)",
                                          "4 (Reserved)",
                                          "5 (Reserved)",
                                          "6 (Reserved)",
                                          "7 (Reserved)",
                                          "8 (Reserved)",
                                          "9 (Reserved)",
                                          "10 (Reserved)",
                                          "11 (Reserved)",
                                          "12 (Reserved)",
                                          "13 (Reserved)",
                                          "14 (Reserved)",
                                          "15 (Reserved)",
                                          "16 (Position)",
                                          "17 (Position)",
                                          "18 (Position)",
                                          "19 (Position)",
                                          "20 (Position)",
                                          "21 (Position)",
                                          "22 (Position)",
                                          "23 (Position)",
                                          "24 (Position)",
                                          "25 (Position)",
                                          "26 (Position)",
                                          "27 (Position)",
                                          "28 (Position)",
                                          "29 (Position)",
                                          "30 (Position)",
                                          "31 (Position)",
                                          "32 (Reserved)",
                                          "33 (Reserved)",
                                          "34 (Reserved)",
                                          "35 (Reserved)",
                                          "36 (Reserved)",
                                          "37 (Reserved)",
                                          "38 (Reserved)",
                                          "39 (Reserved)",
                                          "40 (Reserved)",
                                          "41 (Reserved)",
                                          "42 (Reserved)",
                                          "43 (Reserved)",
                                          "44 (Reserved)",
                                          "45 (Reserved)",
                                          "46 (Reserved)",
                                          "47 (Reserved)",
                                          "48 (Reserved)",
                                          "49 (Reserved)",
                                          "50 (Reserved)",
                                          "51 (Reserved)",
                                          "52 (Reserved)",
                                          "53 (Reserved)",
                                          "54 (Reserved)",
                                          "55 (Reserved)",
                                          "56 (Reserved)",
                                          "57 (Reserved)",
                                          "58 (Reserved)",
                                          "59 (Reserved)",
                                          "60 (Reserved)",
                                          "61 (Reserved)",
                                          "62 (Reserved)",
                                          "63 (Reserved)",
                                          "64 (Reserved)",
                                          "65 (Reserved)",
                                          "66 (Reserved)",
                                          "67 (Reserved)",
                                          "68 (Reserved)",
                                          "69 (Reserved)",
                                          "70 (Reserved)",
                                          "71 (Reserved)",
                                          "72 (Reserved)",
                                          "73 (Reserved)",
                                          "74 (Reserved)",
                                          "75 (Reserved)",
                                          "76 (Reserved)",
                                          "77 (Reserved)",
                                          "78 (Reserved)",
                                          "79 (Reserved)",
                                          "80 (Reserved)",
                                          "81 (Reserved)",
                                          "82 (Reserved)",
                                          "83 (Reserved)",
                                          "84 (Reserved)",
                                          "85 (Reserved)",
                                          "86 (Reserved)",
                                          "87 (Reserved)",
                                          "88 (Reserved)",
                                          "89 (Reserved)",
                                          "90 (Reserved)",
                                          "91 (Reserved)",
                                          "92 (Reserved)",
                                          "93 (Reserved)",
                                          "94 (Reserved)",
                                          "95 (Reserved)",
                                          "96 (Reserved)",
                                          "97 (Reserved)",
                                          "98 (Reserved)",
                                          "99 (Reserved)",
                                          "100 (Reserved)",
                                          "101 (Reserved)",
                                          "102 (Reserved)",
                                          "103 (Reserved)",
                                          "104 (Reserved)",
                                          "105 (Reserved)",
                                          "106 (Reserved)",
                                          "107 (Reserved)",
                                          "108 (Reserved)",
                                          "109 (Reserved)",
                                          "110 (Reserved)",
                                          "111 (Reserved)",
                                          "112 (Reserved)",
                                          "113 (Reserved)",
                                          "114 (Reserved)",
                                          "115 (Reserved)",
                                          "116 (Reserved)",
                                          "117 (Reserved)",
                                          "118 (Reserved)",
                                          "119 (Reserved)",
                                          "120 (Reserved)",
                                          "121 (Reserved)",
                                          "122 (Reserved)",
                                          "123 (Reserved)",
                                          "124 (Reserved)",
                                          "125 (Reserved)",
                                          "126 (Reserved)",
                                          "127 (Reserved)",
                                          "128",
                                          "129",
                                          "130",
                                          "131",
                                          "132",
                                          "133",
                                          "134",
                                          "135",
                                          "136",
                                          "137",
                                          "138",
                                          "139",
                                          "140",
                                          "141",
                                          "142",
                                          "143",
                                          "144",
                                          "145",
                                          "146",
                                          "147",
                                          "148",
                                          "149",
                                          "150",
                                          "151",
                                          "152",
                                          "153",
                                          "154",
                                          "155",
                                          "156",
                                          "157",
                                          "158",
                                          "159",
                                          "160",
                                          "161",
                                          "162",
                                          "163",
                                          "164",
                                          "165",
                                          "166",
                                          "167",
                                          "168",
                                          "169",
                                          "170",
                                          "171",
                                          "172",
                                          "173",
                                          "174",
                                          "175",
                                          "176",
                                          "177",
                                          "178",
                                          "179",
                                          "180",
                                          "181",
                                          "182",
                                          "183",
                                          "184",
                                          "185",
                                          "186",
                                          "187",
                                          "188",
                                          "189",
                                          "190",
                                          "191",
                                          "192",
                                          "193",
                                          "194",
                                          "195",
                                          "196",
                                          "197",
                                          "198",
                                          "199",
                                          "200",
                                          "201",
                                          "202",
                                          "203",
                                          "204",
                                          "205",
                                          "206",
                                          "207",
                                          "208",
                                          "209",
                                          "210",
                                          "211",
                                          "212",
                                          "213",
                                          "214",
                                          "215",
                                          "216",
                                          "217",
                                          "218",
                                          "219",
                                          "220",
                                          "221",
                                          "222",
                                          "223",
                                          "224",
                                          "225",
                                          "226",
                                          "227",
                                          "228",
                                          "229",
                                          "230",
                                          "231",
                                          "232",
                                          "233",
                                          "234",
                                          "235",
                                          "236",
                                          "237",
                                          "238",
                                          "239",
                                          "240",
                                          "241",
                                          "242",
                                          "243",
                                          "244",
                                          "245",
                                          "246",
                                          "247",
                                          "248",
                                          "249",
                                          "250",
                                          "251",
                                          "252",
                                          "253",
                                          "254",
                                          "255"};

//
inline constexpr std::array consist_labels{
  "Not Part", "Leading", "Middle", "Rear"};

//
template<typename T>
requires(std::integral<T> || std::floating_point<T>)
constexpr T random_interval(T min = std::numeric_limits<T>::min(),
                            T max = std::numeric_limits<T>::max()) {
  std::mt19937 gen{std::random_device{}()};
  if constexpr (std::integral<T>) {
    std::uniform_int_distribution<T> dis{min, max};
    return dis(gen);
  } else {
    std::uniform_real_distribution<T> dis{min, max};
    return dis(gen);
  }
}

dcc::Address random_loco_address();
dcc::Address random_basic_accessory_address();
dcc::Address random_extended_accessory_address();

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
                 ImGuiInputTextFlags flags = ImGuiInputTextFlags_None,
                 int numbering_offset = 0);

} // namespace ImGui
