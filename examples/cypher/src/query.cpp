#include "query.hpp"

#if defined(__EMSCRIPTEN__)
#  include <emscripten.h>
EM_JS(char const*, get_query, (), {
  return stringToNewUTF8(window.location.search);
});
#else
char const* get_query() { return "?packets=FF00FF_3A1C71_12AB34"; }
#endif

void query(State& state) {
  auto const str{get_query()};
  printf("Query string: %s\n", str);
}
