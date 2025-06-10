#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <dcc/dcc.hpp>

template<typename T>
struct TxMock : dcc::tx::CrtpBase<TxMock<T>, T> {
  MOCK_METHOD(void, trackOutputs, (bool, bool));
  MOCK_METHOD(void, packetEnd, ());
  MOCK_METHOD(void, biDiStart, ());
  MOCK_METHOD(void, biDiChannel1, ());
  MOCK_METHOD(void, biDiChannel2, ());
  MOCK_METHOD(void, biDiEnd, ());
};
