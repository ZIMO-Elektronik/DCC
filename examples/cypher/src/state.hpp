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
  dcc::tx::Config cfg{};

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
    dcc::tx::Config cfg{};
    bool show{true};
  };

  std::list<Packet> packets{};

  //
  struct Datagram {
    dcc::bidi::Datagram<> bytes{};
    std::vector<std::string> desc{};
    bool show{true};
  };

  std::list<Datagram> datagrams{};
};
