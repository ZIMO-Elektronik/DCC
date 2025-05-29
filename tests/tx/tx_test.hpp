#pragma once

#include <gtest/gtest.h>
#include "tx_mock.hpp"

using namespace ::testing;

// Transmit test fixture
struct TxTest : ::testing::Test {
protected:
  TxTest();
  virtual ~TxTest();

  dcc::tx::Config _cfg{.num_preamble = DCC_TX_MIN_PREAMBLE_BITS + 1u,
                       .bit1_duration = 59u,
                       .bit0_duration = 113u};
  NiceMock<TxMock<dcc::Packet>> _packet_mock;
  NiceMock<TxMock<dcc::tx::Timings>> _timings_mock;
};
