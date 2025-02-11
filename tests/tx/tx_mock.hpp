#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <dcc/dcc.hpp>

struct TxMock : dcc::tx::CrtpBase<TxMock> {
  MOCK_METHOD(void, trackOutputs, (bool, bool));
  MOCK_METHOD(void, biDiStart, ());
  MOCK_METHOD(void, biDiChannel1, ());
  MOCK_METHOD(void, biDiChannel2, ());
  MOCK_METHOD(void, biDiEnd, ());
};
