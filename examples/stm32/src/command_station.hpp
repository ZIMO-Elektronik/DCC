#pragma once

#include <dcc/dcc.hpp>

struct CommandStation : dcc::tx::CrtpBase<CommandStation> {
  friend dcc::tx::CrtpBase<CommandStation>;

private:
  // Write track outputs
  void trackOutputs(bool N, bool P);

  // BiDi start
  void biDiStart();

  // BiDi channel 1
  void biDiChannel1();

  // BiDi channel 2
  void biDiChannel2();

  // BiDi end
  void biDiEnd();
};
