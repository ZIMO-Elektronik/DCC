#pragma once

#include <gtest/gtest.h>
#include "tx_mock.hpp"

using namespace ::testing;

// Transmit test fixture
struct TxTest : ::testing::Test {
protected:
  TxTest();
  virtual ~TxTest();

  NiceMock<TxMock> _mock;
};
