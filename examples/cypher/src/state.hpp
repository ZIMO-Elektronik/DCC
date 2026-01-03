#pragma once

#include <imgui.h>
#include <dcc/dcc.hpp>
#include <list>
#include <string>
#include <tuple>
#include <vector>

struct State {
  // Window flags
  struct {
    bool show_config{};
    bool show_packet_builder{};
    bool show_datagram_builder{};
    bool show_packets{};
    bool show_datagrams{};
    bool show_about{};
  } windows{};

  // DCC config (extended by BiDi bit duration)
  struct Config : dcc::tx::Config {
    friend constexpr bool operator==(Config const& lhs,
                                     Config const& rhs) = default;
    uint8_t bidibit_duration{60u};
  } cfg{};

  // DCC packet with address and meta data for plots
  struct Packet {
    dcc::Address addr{};
    dcc::Packet bytes{};
    std::vector<std::string> desc_strs{};
    std::string pattern_str{};
    struct {
      std::vector<double> t{};
      std::vector<double> p{};
      std::vector<double> n{};
      std::vector<std::tuple<ImColor, std::string>> highlights{};
      std::vector<std::tuple<double, ImColor, std::string>> tags{};
    } plots{};
    Config cfg{};
    bool show{true};
  };

  // Packets
  std::list<Packet> packets{};

  // DCC datagram with address and meta data for plots
  struct Datagram {
    dcc::Address addr{};
    dcc::bidi::Datagram<> bytes{};
    std::vector<std::string> desc_strs{};
    std::string pattern_str{};
    struct {
      std::vector<double> t_p{};
      std::vector<double> p{};
      std::vector<double> t_b{};
      std::vector<double> b{};
      std::vector<std::tuple<ImColor, std::string>> highlights{};
      std::vector<std::tuple<double, ImColor, std::string>> tags{};
    } plots{};
    Config cfg{};
    bool show{true};
  };

  // Datagrams
  std::list<Datagram> datagrams{};
};

// Global state
inline State state{.windows = {.show_config = true,
                               .show_packet_builder = true,
                               .show_datagram_builder = true,
                               .show_packets = true,
                               .show_datagrams = true,
                               .show_about = false}};
