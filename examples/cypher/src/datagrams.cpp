#include "datagrams.hpp"
#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>
#include "state.hpp"
#include "utility.hpp"

#define START_COL ImPlot::GetColormapColor(2) // green
#define STOP_COL ImPlot::GetColormapColor(9)  // red
#define TCS_COL ImPlot::GetColormapColor(8)   // white
#define TTS1_COL ImPlot::GetColormapColor(5)  // yellow
#define TTC1_COL TTS1_COL                     // yellow
#define TTS2_COL ImPlot::GetColormapColor(4)  // orange
#define TTC2_COL TTS2_COL                     // orange
#define TCE_COL ImPlot::GetColormapColor(3)   // violet

namespace {

using namespace dcc::bidi;

constexpr auto tts1{static_cast<double>(Timing::TTS1)};
constexpr auto tts2{static_cast<double>(Timing::TTS2)};

namespace eval {

// clang-format off
void eval(State::Datagram& datagram);
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

void eval(State::Datagram& datagram) {
  if (!empty(datagram.desc_strs) && state.cfg == datagram.cfg) return;

  // Make a copy of global config to know when to rebuild
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
    datagram.plots.t_p.push_back(
      static_cast<double>(i * datagram.cfg.bidibit_duration));
    datagram.plots.p.push_back(bit);
    datagram.plots.t_p.push_back(
      static_cast<double>(i * datagram.cfg.bidibit_duration));
    datagram.plots.p.push_back(!bit);
  }

  // BiDi
  static constexpr auto scale{0.5};
  datagram.plots.t_b.push_back(0.0);
  datagram.plots.b.push_back(1.0 * scale);
  for (auto i{0uz}; i < bundled_channels_size; ++i) {
    auto offset{i < 2uz ? tts1 : tts2 - channel1_size * 10u * 4u};
    offset = offset + static_cast<double>(i * 10u * 4u);

    auto const b{datagram.bytes[i]};
    if (!b) continue;

    // Startbit
    datagram.plots.t_b.push_back(offset + static_cast<double>(0u * 4u));
    datagram.plots.b.push_back(1.0 * scale);
    datagram.plots.t_b.push_back(offset + static_cast<double>(0u * 4u));
    datagram.plots.b.push_back(0.0);
    datagram.plots.t_b.push_back(offset + static_cast<double>(1u * 4u));
    datagram.plots.b.push_back(0.0);

    bool bit;
    for (auto j{1uz}; j <= CHAR_BIT; ++j) {
      bit = static_cast<bool>(b & (1u << (j - 1u)));
      datagram.plots.t_b.push_back(offset + static_cast<double>(j * 4u));
      datagram.plots.b.push_back(bit * scale);
      datagram.plots.t_b.push_back(offset + static_cast<double>((j + 1) * 4u));
      datagram.plots.b.push_back(bit * scale);
    }

    // Stopbit
    datagram.plots.t_b.push_back(offset + static_cast<double>(9u * 4u));
    datagram.plots.b.push_back(bit * scale);
    datagram.plots.t_b.push_back(offset + static_cast<double>(9u * 4u));
    datagram.plots.b.push_back(1.0 * scale);
    datagram.plots.t_b.push_back(offset + static_cast<double>(10u * 4u));
    datagram.plots.b.push_back(1.0 * scale);
  }
  datagram.plots.t_b.push_back(datagram.plots.t_p.back());
  datagram.plots.b.push_back(1.0 * scale);

  dissector(datagram);
  highlights(datagram);
  tags(datagram);
}

// Annotate datagram
void dissector(State::Datagram& datagram) {
  datagram.desc_strs.push_back(
    is_mob_address(datagram.addr) ? "Loco (MOB)" : "Accessory (STAT)");
  Dissector dissector{datagram.bytes, datagram.addr};
  for (auto it{begin(dissector)}; it != end(dissector); ++it) {
    auto const& dg{*it};
    if (auto const ack{get_if<Ack>(&dg)}) {
      datagram.desc_strs.push_back("ACK");
    } else if (auto const nak{get_if<Nak>(&dg)}) {
      datagram.desc_strs.push_back("NAK");
    } else if (auto const pom{get_if<app::Pom>(&dg)}) {
      datagram.desc_strs.push_back("app:pom");
      datagram.desc_strs.back() += std::format("\n- CV={}", pom->d);
    } else if (auto const adr_high{get_if<app::AdrHigh>(&dg)}) {
      datagram.desc_strs.push_back("app:adr_high");
      datagram.desc_strs.back() +=
        std::format("\n- Address High Byte={:08b} ", adr_high->d);
    } else if (auto const adr_low{get_if<app::AdrLow>(&dg)}) {
      datagram.desc_strs.push_back("app:adr_low");
      datagram.desc_strs.back() +=
        std::format("\n- Address Low Byte={:08b} ", adr_low->d);
    } else if (auto const info1{get_if<app::Info1>(&dg)}) {
      datagram.desc_strs.push_back("app:info1");
      datagram.desc_strs.back() += std::format(
        "\n- Track Polarity={}", info1->d & app::Info1::TrackPolarity ? 1 : 0);
      datagram.desc_strs.back() += std::format(
        "\n- East-West={}", info1->d & app::Info1::EastWest ? 1 : 0);
      datagram.desc_strs.back() +=
        std::format("\n- Driving={}", info1->d & app::Info1::Driving ? 1 : 0);
      datagram.desc_strs.back() +=
        std::format("\n- Consist={}", info1->d & app::Info1::Consist ? 1 : 0);
      datagram.desc_strs.back() +=
        std::format("\n- Addressing Request={}",
                    info1->d & app::Info1::AddressingRequest ? 1 : 0);
    } else if (auto const ext{get_if<app::Ext>(&dg)}) {
      datagram.desc_strs.push_back("app:ext");
      datagram.desc_strs.back() +=
        std::format("\n- {}",
                    ext->t ? app_ext_type_labels[ext->t]
                           : "Address Only (TTTT = 0000-0111)");
      datagram.desc_strs.back() += std::format("\n- Position={}", ext->p);
    } else if (auto const info{get_if<app::Info>(&dg)}) {
      datagram.desc_strs.push_back("app:info");
      datagram.desc_strs.push_back("\\todo");
    } else if (auto const dyn{get_if<app::Dyn>(&dg)}) {
      datagram.desc_strs.push_back("app:dyn");
      datagram.desc_strs.back() += std::format("\n- D={}", dyn->d);
      datagram.desc_strs.back() += std::format("\n- X={}", dyn->x);
      datagram.desc_strs.back() += std::format(
        " ({})",
        is_mob_address(datagram.addr) ? mob_app_dyn_labels[dyn->x + 1u]
                                      : stat_app_dyn_labels[dyn->x + 1u]);
      switch (dyn->x) {
        // Speed
        case 0u: [[fallthrough]];
        case 1u:
          datagram.desc_strs.back() +=
            std::format("\n- Speed={}km/h", dyn->d + (dyn->x ? 256u : 0u));
          break;
        // Load or speed steps
        case 2u:
          datagram.desc_strs.back() +=
            std::format("\n- {}={}",
                        dyn->d & 0x80u ? "Speed Step" : "Load",
                        dyn->d & 0x7Fu);
          break;
        // RailCom version
        case 3u:
          datagram.desc_strs.back() += std::format(
            "\n- Version={}.{}", (dyn->d >> 4u) & 0x0Fu, dyn->d & 0x0Fu);
          break;
        // Change flags
        case 4u:
          datagram.desc_strs.back() += std::format("\n- Flags={:08b} ", dyn->d);
          break;
        // Flag register
        case 5u:
          datagram.desc_strs.back() +=
            std::format("\n- Register={:08b} ", dyn->d);
          break;
        // Input register
        case 6u:
          datagram.desc_strs.back() +=
            std::format("\n- Register={:08b} ", dyn->d);
          break;
        // QoS
        case 7u:
          datagram.desc_strs.back() += std::format("\n- QoS={}%", dyn->d);
          break;
        // Container
        case 8u: [[fallthrough]];
        case 9u: [[fallthrough]];
        case 10u: [[fallthrough]];
        case 11u: [[fallthrough]];
        case 12u: [[fallthrough]];
        case 13u: [[fallthrough]];
        case 14u: [[fallthrough]];
        case 15u: [[fallthrough]];
        case 16u: [[fallthrough]];
        case 17u: [[fallthrough]];
        case 18u: [[fallthrough]];
        case 19u:
          datagram.desc_strs.back() += std::format("\n- Level={}%", dyn->d);
          break;
        // Position (app:ext)
        case 20u:
          // Second app:dyn
          if (auto it2{it}; ++it2 != end(dissector)) {
            auto const dg2{*it2};
            if (auto const dyn_2{get_if<app::Dyn>(&dg2)}) {
              it = it2;
              datagram.desc_strs.push_back("app:dyn");
              datagram.desc_strs.back() += std::format("\n- D={}", dyn_2->d);
              datagram.desc_strs.back() += std::format("\n- X={}", dyn_2->x);
              datagram.desc_strs.back() +=
                std::format(" ({})",
                            is_mob_address(datagram.addr)
                              ? mob_app_dyn_labels[dyn_2->x + 1u]
                              : stat_app_dyn_labels[dyn_2->x + 1u]);
              datagram.desc_strs.back() +=
                std::format("\n- Position={}", (dyn_2->d << 8u) | dyn->d);
            }
          }
          // Only one
          else
            datagram.desc_strs.back() += std::format("\n- Position={}", dyn->d);
          break;
        // Status and alarm messages
        case 21u:
          datagram.desc_strs.back() +=
            std::format("\n- Related to DV={}", dyn->d & ztl::mask<6u> ? 1 : 0);
          // Related to DV
          if (dyn->d & ztl::mask<6u>) {
            datagram.desc_strs.back() +=
              std::format("\n- Alarm={}", dyn->d & ztl::mask<7u> ? 1 : 0);
            datagram.desc_strs.back() +=
              std::format("\n- DV={}", dyn->d & 0x3Fu);
          }
          // Pre-defined
          else {
            // MOB
            if (is_mob_address(datagram.addr)) switch (dyn->d) {
                case 128u: [[fallthrough]];
                case 129u: [[fallthrough]];
                case 130u:
                  datagram.desc_strs.back() += std::format(
                    "\n- Alarm={}",
                    mob_app_dyn_status_and_alarm_messages_labels[dyn->d -
                                                                 128uz]);
                  break;
                default: datagram.desc_strs.back() += "\n- Alarm=?"; break;
              }
            // STAT
            else switch (dyn->d) {
                case 128u: [[fallthrough]];
                case 129u: [[fallthrough]];
                case 130u: [[fallthrough]];
                case 131u: [[fallthrough]];
                case 132u: [[fallthrough]];
                case 133u: [[fallthrough]];
                case 134u: [[fallthrough]];
                case 135u: [[fallthrough]];
                case 136u:
                  datagram.desc_strs.back() += std::format(
                    "\n- Alarm={}",
                    stat_app_dyn_status_and_alarm_messages_labels[dyn->d -
                                                                  128uz]);
                  break;
                default: datagram.desc_strs.back() += "\n- Alarm=?"; break;
              }
          }
          break;
        // Trip odometer
        case 22u:
          datagram.desc_strs.back() +=
            std::format("\n- Trip Odometer={}", dyn->d);
          break;
        // Maintenance interval
        case 23u:
          datagram.desc_strs.back() += std::format("\n- Interval={}", dyn->d);
          break;
        // Reserved
        case 24u: break;
        case 25u: [[fallthrough]];
        // Temperature
        case 26u:
          datagram.desc_strs.back() +=
            std::format("\n- Temperature={}°C",
                        ztl::lerp<int32_t>(dyn->d, 0u, 255u, -50, 205));
          break;
        // Direction status byte
        case 27u:
          datagram.desc_strs.back() +=
            std::format("\n- Direction={}", dyn->d & ztl::mask<0u> ? 1 : 0);
          datagram.desc_strs.back() +=
            std::format("\n- East-West={}", dyn->d & ztl::mask<1u> ? 1 : 0);
          datagram.desc_strs.back() += std::format(
            "\n- Direction Control={}", dyn->d & ztl::mask<2u> ? 1 : 0);
          datagram.desc_strs.back() += std::format(
            "\n- Direction Change={}", dyn->d & ztl::mask<3u> ? 1 : 0);
          datagram.desc_strs.back() +=
            std::format("\n- Hide UI={}", dyn->d & ztl::mask<4u> ? 1 : 0);
          datagram.desc_strs.back() += std::format(
            "\n- East-West Inverted={}", dyn->d & ztl::mask<5u> ? 1 : 0);
          break;
        // Reserved
        case 28u: [[fallthrough]];
        case 29u: [[fallthrough]];
        case 30u: [[fallthrough]];
        case 31u: [[fallthrough]];
        case 32u: [[fallthrough]];
        case 33u: break;
        // Control deviation
        case 34u:
          datagram.desc_strs.back() +=
            std::format("\n- Control Deviation={}", dyn->d);
          break;
        case 35u: [[fallthrough]];
        case 36u: [[fallthrough]];
        case 37u: [[fallthrough]];
        case 38u: [[fallthrough]];
        case 39u: [[fallthrough]];
        case 40u: [[fallthrough]];
        case 41u: [[fallthrough]];
        case 42u: [[fallthrough]];
        case 43u: [[fallthrough]];
        case 44u: [[fallthrough]];
        case 45u: break;
        // Track voltage
        case 46u:
          datagram.desc_strs.back() +=
            std::format("\n- Track Voltage={:.1f}V", 5 + dyn->d * 0.1);
          break;
        // Stopping distance
        case 47u:
          datagram.desc_strs.back() +=
            std::format("\n- Stopping Distance={}m", dyn->d * 4u);
          break;
        // Reserved
        case 48u: [[fallthrough]];
        case 49u: [[fallthrough]];
        case 50u: [[fallthrough]];
        case 51u: [[fallthrough]];
        case 52u: [[fallthrough]];
        case 53u: [[fallthrough]];
        case 54u: [[fallthrough]];
        case 55u: [[fallthrough]];
        case 56u: [[fallthrough]];
        case 57u: [[fallthrough]];
        case 58u: [[fallthrough]];
        case 59u: [[fallthrough]];
        case 60u: [[fallthrough]];
        case 61u: [[fallthrough]];
        case 62u: [[fallthrough]];
        case 63u: break;
      }
    } else if (auto const xpom{get_if<app::Xpom>(&dg)}) {
      datagram.desc_strs.push_back("app:xpom");
      datagram.desc_strs.back() +=
        std::format("\n- Sequence Number={:02b}", xpom->ss);
      datagram.desc_strs.back() += std::format("\n- CV[0]={}", xpom->d[0uz]);
      datagram.desc_strs.back() += std::format("\n- CV[1]={}", xpom->d[1uz]);
      datagram.desc_strs.back() += std::format("\n- CV[2]={}", xpom->d[2uz]);
      datagram.desc_strs.back() += std::format("\n- CV[3]={}", xpom->d[3uz]);
    } else if (auto const cv_auto{get_if<app::CvAuto>(&dg)}) {
      datagram.desc_strs.push_back("app:CV-auto");
      datagram.desc_strs.back() +=
        std::format("\n- CV{}={}", cv_auto->v + 1u, cv_auto->d);
    } else if (auto const block{get_if<app::Block>(&dg)}) {
      datagram.desc_strs.push_back("app:block");
      datagram.desc_strs.push_back("\\todo");
    } else if (auto const search{get_if<app::Search>(&dg)}) {
      datagram.desc_strs.push_back("app:search");
      datagram.desc_strs.back() += std::format("\n- Time={}s", search->d);
    } else if (auto const srq{get_if<app::Srq>(&dg)}) {
      datagram.desc_strs.push_back("app:srq");
      datagram.desc_strs.back() +=
        std::format("\n- {} Accessory={}",
                    srq->d & ztl::mask<11u> ? "Extended" : "Basic",
                    srq->d & 0x7FFu);
    } else if (auto const stat4{get_if<app::Stat4>(&dg)}) {
      datagram.desc_strs.push_back("app:stat4");
      datagram.desc_strs.back() +=
        std::format("\n- R4-R1={}{}{}{}",
                    stat4->d & ztl::mask<6u> ? 1 : 0,
                    stat4->d & ztl::mask<4u> ? 1 : 0,
                    stat4->d & ztl::mask<2u> ? 1 : 0,
                    stat4->d & ztl::mask<0u> ? 1 : 0);
    } else if (auto const stat1{get_if<app::Stat1>(&dg)}) {
      datagram.desc_strs.push_back("app:stat1");
      // Basic accessory
      if (datagram.addr.type == dcc::Address::BasicAccessory) {
        datagram.desc_strs.back() += "\n- Basic Accessory";
        datagram.desc_strs.back() +=
          std::format("\n- Initial State Matches Last Received={}",
                      stat1->d & ztl::mask<6u> ? 1 : 0);
        datagram.desc_strs.back() +=
          std::format("\n- Returned Aspect Based on Feedback={}",
                      stat1->d & ztl::mask<5u> ? 1 : 0);
        datagram.desc_strs.back() +=
          std::format("\n- Aspect={}", stat1->d & 0b1'1111);
      }
      // Extended accessory
      else if (datagram.addr.type == dcc::Address::ExtendedAccessory) {
        datagram.desc_strs.back() += "\n- Extended Accessory";
        datagram.desc_strs.back() +=
          std::format("\n- Initial State Matches Last Received={}",
                      stat1->d & ztl::mask<6u> ? 1 : 0);
        datagram.desc_strs.back() +=
          std::format("\n- Returned Aspect Based on Feedback={}",
                      stat1->d & ztl::mask<5u> ? 1 : 0);
        // Second app:stat1
        if (auto it2{it}; ++it2 != end(dissector)) {
          auto const dg2{*it2};
          if (auto const stat1_2{get_if<app::Stat1>(&dg2)}) {
            it = it2;
            datagram.desc_strs.push_back("app:stat1");
            datagram.desc_strs.back() += "\n- Extended Accessory";
            datagram.desc_strs.back() +=
              std::format("\n- Aspect={}",
                          (stat1_2->d & 0b111) << 5u | (stat1->d & 0b1'1111));
          }
        }
        // No second app:stat1
        else
          datagram.desc_strs.back() +=
            std::format("\n- Aspect={}", stat1->d & 0b1'1111);
      }
    } else if (auto const time{get_if<app::Time>(&dg)}) {
      datagram.desc_strs.push_back("app:time");
      datagram.desc_strs.back() +=
        std::format("\n- Time={}s",
                    time->d & 0x80u ? (1.0 * (time->d & 0x7Fu))
                                    : (0.1 * (time->d & 0x7Fu)));
    } else if (auto const error{get_if<app::Error>(&dg)}) {
      datagram.desc_strs.push_back("app:error");
      datagram.desc_strs.back() +=
        std::format("\n- Additional Errors={}",
                    error->d & app::Error::AdditionalErrors ? 1 : 0);
      switch (error->d & 0x3Fu) {
        case app::Error::None:
          datagram.desc_strs.back() += "\n- Error=None";
          break;
        case app::Error::InvalidCommand:
          datagram.desc_strs.back() += "\n- Error=Invalid Command";
          break;
        case app::Error::Overcurrent:
          datagram.desc_strs.back() += "\n- Error=Overcurrent";
          break;
        case app::Error::Undervoltage:
          datagram.desc_strs.back() += "\n- Error=Undervoltage";
          break;
        case app::Error::Fuse:
          datagram.desc_strs.back() += "\n- Error=Fuse";
          break;
        case app::Error::Overtemperature:
          datagram.desc_strs.back() += "\n- Error=Overtemperature";
          break;
        case app::Error::Feedback:
          datagram.desc_strs.back() += "\n- Error=Feedback";
          break;
        case app::Error::ManualOperation:
          datagram.desc_strs.back() += "\n- Error=Manual Operation";
          break;
        case app::Error::Signal:
          datagram.desc_strs.back() += "\n- Error=Signal";
          break;
        case app::Error::Servo:
          datagram.desc_strs.back() += "\n- Error=Servo";
          break;
        case app::Error::Internal:
          datagram.desc_strs.back() += "\n- Error=Internal";
          break;
      }
    } else if (auto const test{get_if<app::Test>(&dg)}) {
      datagram.desc_strs.push_back("app:test");
      datagram.desc_strs.push_back("\\todo");
    }
  }
}

// Generate highlights for plot
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

  // Caret index
  size_t c{};

  for (auto i{0uz}; i < bundled_channels_size; ++i) {
    auto offset{i < 2uz ? tts1 : tts2 - channel1_size * 10u * 4u};
    offset += static_cast<double>(i * 10u * 4u);

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

// Generate tags for plot
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

// Tab
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

// Description node
void description(State::Datagram& datagram) {
  for (auto const& desc : datagram.desc_strs)
    ImGui::BulletText("%s", desc.c_str());
}

// Data node
void data(State::Datagram& datagram) {
  // This is beyond stupid, but have you tried doing that shit with algorithms?
  int first{-1};
  int last{-1};
  for (auto i{0}; i < ssize(datagram.bytes); ++i) {
    if (!datagram.bytes[static_cast<size_t>(i)]) continue;
    if (first == -1) first = i;
    last = i;
  }
  ImGui::BinaryTable(UNIQUE_LABEL(),
                     data(datagram.bytes) + first,
                     last - first + 1,
                     ImGuiInputTextFlags_ReadOnly,
                     first);
}

// Plot node
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
      // Channel 1
      if (mouse.x >= tts1 && mouse.x <= tts1 + 10u * 4u * channel1_size)
        b = datagram.bytes[static_cast<size_t>((mouse.x - tts1) / (10u * 4u))];
      // Channel 2
      else if (mouse.x >= tts2 && mouse.x <= tts2 + 10u * 4u * channel2_size)
        b = datagram.bytes[channel1_size +
                           static_cast<size_t>((mouse.x - tts2) / (10u * 4u))];

      if (auto const it{
            std::ranges::adjacent_find(datagram.plots.t_b,
                                       [x = mouse.x](double lhs, double rhs) {
                                         return x >= lhs && x <= rhs;
                                       })};
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

// Datagrams window
void datagrams() {
  if (!state.windows.show_datagrams) return;

  if (ImGui::Begin("Datagrams", &state.windows.show_datagrams)) {
    if (ImGui::BeginTabBar(UNIQUE_LABEL())) {
      size_t i{};

      for (auto& datagram : state.datagrams) {
        eval::eval(datagram);
        tab::tab(datagram, i++);
      }

      ImGui::EndTabBar();
    }
  }
  ImGui::End();
}
