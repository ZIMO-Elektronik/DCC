#include "query.hpp"
#include "utility.hpp"

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

} // namespace

void query(State& state) {
  std::string url{get_query()};
  printf("Query string: %s\n", url.c_str());

  auto packets{get_query_param(url, "packets")};
  for (auto packet : packets | std::views::split('_')) {
    std::string seq(packet.begin(), packet.end()); // convert view → string
    printf("%s\n", seq.c_str());
  }

  auto datagrams{get_query_param(url, "datagrams")};
  for (auto packet : datagrams | std::views::split('_')) {
    std::string seq(packet.begin(), packet.end()); // convert view → string
    printf("%s\n", seq.c_str());
  }
}
