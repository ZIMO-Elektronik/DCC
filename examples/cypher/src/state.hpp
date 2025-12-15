#pragma once

#include <imgui.h>
#include <dcc/dcc.hpp>
#include <list>
#include <string>
#include <tuple>
#include <vector>

struct State {
  //
  struct {
    bool show_config{};
    bool show_packet_builder{};
    bool show_datagram_builder{};
    bool show_packets{};
    bool show_datagrams{};
    bool show_about{};
  } windows{};

  //
  struct Config : dcc::tx::Config {
    friend constexpr bool operator==(Config const& lhs,
                                     Config const& rhs) = default;
    uint8_t bidibit_duration{60u};
  } cfg{};

  //
  struct Packet {
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

  // Operations mode packets
  std::list<Packet> operations_packets{};

  // Service mode packets
  std::list<Packet> service_packets{};

  //
  struct Datagram {
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

  // Loco datagrams
  std::list<Datagram> loco_datagrams{};

  // Accessory datagrams
  std::list<Datagram> accessory_datagrams{};
};
