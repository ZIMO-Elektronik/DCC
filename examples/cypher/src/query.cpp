#include "query.hpp"
#include <string>
#include "demo.hpp"

namespace {

#if defined(__EMSCRIPTEN__)
#  include <emscripten.h>
EM_JS(char const*, get_query, (), {
  return stringToNewUTF8(window.location.search);
});
EM_JS(char const*, get_url, (), {
  return stringToNewUTF8(window.location.href);
});
#else
char const* get_query() { return ""; }
char const* get_url() { return "https://zimo-elektronik.github.io/DCC/"; }
#endif

//
std::string get_query_param(std::string const& url, std::string const& param) {
  auto const pattern{param + "="};
  auto const start_pos{url.find(pattern)};
  if (start_pos == std::string::npos) return {};
  auto const end_pos{url.find("?", start_pos)};
  return end_pos == std::string::npos
           ? url.substr(start_pos + size(pattern))
           : url.substr(start_pos + size(pattern),
                        end_pos - size(pattern) - 1uz);
}

//
auto to_vector(std::string const& values) {
  std::vector<std::vector<uint8_t>> retval;
  for (auto value : values | std::views::split('_')) {
    if (auto const count{size(value)}; count < 4uz || count % 2uz) continue;
    std::vector<uint8_t> v;
    for (auto i{0uz}; i < size(value); i += 2uz) {
      std::string const hex{value[i], value[i + 1uz]};
      v.push_back(static_cast<uint8_t>(std::stoi(hex, nullptr, 16)));
    }
    retval.push_back(v);
  }
  return retval;
}

//
auto vector2packet(std::vector<uint8_t> v) {
  dcc::Packet retval{};
  std::ranges::copy(v, std::back_inserter(retval));
  return retval;
}

//
auto vector2datagram(std::vector<uint8_t> v) {
  dcc::bidi::Datagram<> retval{};
  std::ranges::copy_n(
    cbegin(v), std::min(ssize(v), ssize(retval)), begin(retval));
  return retval;
}

} // namespace

//
void query(State& state) { from_query(state, get_query()); }

//
void from_query(State& state, std::string const& url) {
  if (auto const param{get_query_param(url, "demo")}; size(param)) {
    state.op_packets.clear();
    state.serv_packets.clear();
    state.loco_datagrams.clear();
    demo(state);
    return;
  }

  if (auto const param{get_query_param(url, "op_packets")}; size(param)) {
    state.op_packets.clear();
    auto const vs{to_vector(param)};
    for (auto const& v : vs)
      state.op_packets.push_back({.bytes = vector2packet(v)});
  }

  if (auto const param{get_query_param(url, "serv_packets")}; size(param)) {
    state.serv_packets.clear();
    auto const vs{to_vector(param)};
    for (auto const& v : vs)
      state.serv_packets.push_back({.bytes = vector2packet(v)});
  }

  if (auto const param{get_query_param(url, "loco_datagrams")}; size(param)) {
    state.loco_datagrams.clear();
    auto const vs{to_vector(param)};
    for (auto const& v : vs)
      state.loco_datagrams.push_back({.bytes = vector2datagram(v)});
  }

  if (auto const param{get_query_param(url, "accy_datagrams")}; size(param)) {
    state.accy_datagrams.clear();
    auto const vs{to_vector(param)};
    for (auto const& v : vs)
      state.accy_datagrams.push_back({.bytes = vector2datagram(v)});
  }
}

//
std::string to_query(State& state) {
  std::string retval{get_url()};

  if (size(state.op_packets)) {
    retval += "?op_packets=";
    for (auto packet : state.op_packets) {
      if (!packet.show) continue;
      for (auto b : packet.bytes) retval += std::format("{:02X}", b);
      retval += '_';
    }
    retval.pop_back();
  }

  if (size(state.serv_packets)) {
    retval += "?serv_packets=";
    for (auto packet : state.serv_packets) {
      if (!packet.show) continue;
      for (auto b : packet.bytes) retval += std::format("{:02X}", b);
      retval += '_';
    }
    retval.pop_back();
  }

  if (size(state.loco_datagrams)) {
    retval += "?loco_datagrams=";
    for (auto datagram : state.loco_datagrams) {
      if (!datagram.show) continue;
      for (auto b : datagram.bytes) retval += std::format("{:02X}", b);
      retval += '_';
    }
    retval.pop_back();
  }

  if (size(state.accy_datagrams)) {
    retval += "?accy_datagrams=";
    for (auto datagram : state.accy_datagrams) {
      if (!datagram.show) continue;
      for (auto b : datagram.bytes) retval += std::format("{:02X}", b);
      retval += '_';
    }
    retval.pop_back();
  }

  return retval;
}
