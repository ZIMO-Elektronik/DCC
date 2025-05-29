#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <dcc/dcc.hpp>

struct TxMock : dcc::tx::CrtpBase<TxMock, dcc::tx::ITimings> {
  MOCK_METHOD(void, biDiStart, ());
  MOCK_METHOD(void, biDiChannel1, ());
  MOCK_METHOD(void, biDiChannel2, ());
  MOCK_METHOD(void, biDiEnd, ());
};

struct TxMockP : dcc::tx::CrtpBase<TxMockP, dcc::tx::IPacket> {
  MOCK_METHOD(void, biDiStart, ());
  MOCK_METHOD(void, biDiChannel1, ());
  MOCK_METHOD(void, biDiChannel2, ());
  MOCK_METHOD(void, biDiEnd, ());
};
