#include "query.hpp"
#include <string>
#include "demo.hpp"

namespace {

#if defined(__EMSCRIPTEN__)
#  include <emscripten.h>
EM_JS(char const*, get_query, (), {
  return stringToNewUTF8(window.location.search);
});
#else
char const* get_query() {
  return "?packets=FF00FF_3A1C71_12AB34?datagrams=FF1234FF";
}
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
  auto const hexval{[](auto c) -> uint8_t {
    if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
    if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10u);
    if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(c - 'a' + 10u);
    return 0u;
  }};
  std::vector<std::vector<uint8_t>> retval;
  for (auto value : values | std::views::split('_')) {
    if (auto const count{size(value)}; count < 2uz || count % 2uz) continue;
    std::vector<uint8_t> v;
    for (auto i{0uz}; i < size(value); i += 2uz) {
      auto const hi{value[i]};
      auto const lo{value[i + 1uz]};
      v.push_back((hexval(hi) << 4u) | hexval(lo));
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

void query(State& state) {
  std::string url{get_query()};

  if (auto const param{get_query_param(url, "demo")}; size(param)) demo(state);

  if (auto const param{get_query_param(url, "packets")}; size(param)) {
    auto const vs{to_vector(param)};
    for (auto const& v : vs)
      state.packets.push_back({.bytes = vector2packet(v)});
  }

  if (auto const param{get_query_param(url, "datagrams")}; size(param)) {
    auto const vs{to_vector(param)};
    for (auto const& v : vs)
      state.datagrams.push_back({.bytes = vector2datagram(v)});
  }
}
