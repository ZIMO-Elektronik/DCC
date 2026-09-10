#pragma once

#include <dcc/dcc.hpp>

struct CommandStation : dcc::tx::PacketsBase {
  friend dcc::tx::PacketsBase;

private:
  // Write track outputs
  void trackOutputs(bool N, bool P);
};
