#pragma once

#include <dcc/dcc.hpp>

struct CommandStation : dcc::tx::Base<> {
  friend dcc::tx::Base<>;

private:
  // BiDi start
  void biDiStart();

  // BiDi channel 1
  void biDiChannel1();

  // BiDi channel 2
  void biDiChannel2();

  // BiDi end
  void biDiEnd();
};
