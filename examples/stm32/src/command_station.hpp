#pragma once

#include <dcc/dcc.hpp>

struct CommandStation : dcc::tx::CrtpBase<CommandStation> {
  friend dcc::tx::CrtpBase<CommandStation>;

private:
  // Write track outputs
  void trackOutputs(bool N, bool P);
};
