#pragma once

#include <gtest/gtest.h>
#include "tx_mock.hpp"

using namespace ::testing;

// Transmit test fixture
struct TxTest : ::testing::Test {
public:
  bool EvalPacket(dcc::tx::Timings timings,
                  dcc::Packet packet,
                  dcc::tx::Timings::size_type& offset,
                  dcc::tx::Config cfg = {});
  bool EvalPreamble(dcc::tx::Timings timings,
                    dcc::tx::Timings::size_type& offset,
                    dcc::tx::Config cfg = {});
  bool EvalByte(dcc::tx::Timings timings,
                uint8_t byte,
                dcc::tx::Timings::size_type& offset,
                dcc::tx::Config cfg = {});
  bool EvalStartBit(dcc::tx::Timings timings,
                    dcc::tx::Timings::size_type& offset,
                    dcc::tx::Config cfg = {});

  bool EvalStopBit(dcc::tx::Timings timings,
                   dcc::tx::Timings::size_type& offset,
                   dcc::tx::Config cfg = {});
  bool EvalBiDi(dcc::tx::Timings timings,
                dcc::tx::Timings::size_type& offset,
                dcc::tx::Config cfg = {});

  dcc::tx::Timings TransmitT(int lengthOfData, dcc::tx::Config cfg = {});
  dcc::tx::Timings TransmitP(int lengthOfData, dcc::tx::Config cfg = {});

protected:
  TxTest();
  virtual ~TxTest();

  NiceMock<TxMock> _mock;
  NiceMock<TxMockP> _mockp;
};
