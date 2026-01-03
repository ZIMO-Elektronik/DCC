#include "query.hpp"
#include <string>
#include "demo.hpp"
#include "state.hpp"

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

// Get query parameter from URL
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

// Split string to vector of strings
auto to_str_vector(std::string const& str) {
  std::vector<std::string> retval;
  for (auto&& sub : str | std::views::split('_'))
    retval.emplace_back(begin(sub), end(sub));
  return retval;
}

// Convert string to address
dcc::Address str2addr(std::string const& str) {
  dcc::Address retval;
  switch (str[0uz]) {
    case 'u': retval.type = dcc::Address::UnknownService; break;
    case 'b': retval.type = dcc::Address::Broadcast; break;
    case 's': retval.type = dcc::Address::BasicLoco; break;
    case 'a': retval.type = dcc::Address::BasicAccessory; break;
    case 'x': retval.type = dcc::Address::ExtendedAccessory; break;
    case 'l': retval.type = dcc::Address::ExtendedLoco; break;
    case 'r': retval.type = dcc::Address::Reserved; break;
    case 't': retval.type = dcc::Address::DataTransfer; break;
    case 'e': retval.type = dcc::Address::AutomaticLogon; break;
    case 'i': retval.type = dcc::Address::Idle; break;
  }
  retval.value =
    static_cast<dcc::Address::value_type>(std::stoi(&str[1uz], nullptr, 16));
  return retval;
}

// Convert string to packet
State::Packet str2packet(std::string const& str) {
  dcc::Packet bytes;
  for (auto i{6uz}; i < size(str) - 1uz; i += 2uz) {
    std::string const hex{str[i], str[i + 1uz]};
    bytes.push_back(static_cast<uint8_t>(std::stoi(hex, nullptr, 16)));
  }
  return {.addr = str2addr(str), .bytes = bytes};
}

// Convert string to datagram
State::Datagram str2datagram(std::string const& str) {
  dcc::bidi::Datagram<> bytes;
  size_t count{};
  for (auto i{6uz}; i < size(str) - 1uz; i += 2uz) {
    std::string const hex{str[i], str[i + 1uz]};
    bytes[count++] = static_cast<uint8_t>(std::stoi(hex, nullptr, 16));
  }
  return {.addr = str2addr(str), .bytes = bytes};
}

// Convert address to string
std::string addr2str(dcc::Address addr) {
  std::string retval;
  switch (addr.type) {
    case dcc::Address::UnknownService: retval += 'u'; break;
    case dcc::Address::Broadcast: retval += 'b'; break;
    case dcc::Address::BasicLoco: retval += 's'; break;
    case dcc::Address::BasicAccessory: retval += 'a'; break;
    case dcc::Address::ExtendedAccessory: retval += 'x'; break;
    case dcc::Address::ExtendedLoco: retval += 'l'; break;
    case dcc::Address::Reserved: retval += 'r'; break;
    case dcc::Address::DataTransfer: retval += 't'; break;
    case dcc::Address::AutomaticLogon: retval += 'e'; break;
    case dcc::Address::Idle: retval += 'i'; break;
  }
  retval += std::format("{:04X}-", addr.value);
  return retval;
}

} // namespace

// Query
void query() { from_query(get_query()); }

// Fill state from query
void from_query(std::string const& url) {
  if (auto const param{get_query_param(url, "demo")}; size(param))
    return demo();

  if (auto const param{get_query_param(url, "packets")}; size(param)) {
    state.packets.clear();
    auto const strs{to_str_vector(param)};
    for (auto const& str : strs) state.packets.push_back(str2packet(str));
  }

  if (auto const param{get_query_param(url, "datagrams")}; size(param)) {
    state.datagrams.clear();
    auto const strs{to_str_vector(param)};
    for (auto const& str : strs) state.datagrams.push_back(str2datagram(str));
  }
}

// Generate query from state
std::string to_query() {
  std::string retval{get_url()};

  if (size(state.packets)) {
    retval += "?packets=";
    for (auto packet : state.packets) {
      if (!packet.show) continue;
      retval += addr2str(packet.addr);
      for (auto b : packet.bytes) retval += std::format("{:02X}", b);
      retval += '_';
    }
    retval.pop_back();
  }

  if (size(state.datagrams)) {
    retval += "?datagrams=";
    for (auto datagram : state.datagrams) {
      if (!datagram.show) continue;
      retval += addr2str(datagram.addr);
      for (auto b : datagram.bytes) retval += std::format("{:02X}", b);
      retval += '_';
    }
    retval.pop_back();
  }

  return retval;
}
