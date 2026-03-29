#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <dcc/dcc.hpp>

struct RxMock : dcc::rx::CrtpBase<RxMock> {
  MOCK_METHOD(void, direction, (uint16_t, int32_t), ());
  MOCK_METHOD(void, speed, (uint16_t, int32_t), ());
  MOCK_METHOD(void, function, (uint16_t, uint32_t, uint32_t), ());
  MOCK_METHOD(void, serviceModeHook, (bool), ());
  MOCK_METHOD(void, serviceAck, (), ());
  MOCK_METHOD(uint8_t, readCv, (uint32_t), ());
  MOCK_METHOD(uint8_t, readCv, (uint32_t, uint8_t), ());
  MOCK_METHOD(uint8_t, writeCv, (uint32_t, uint8_t), ());
  MOCK_METHOD(bool, readCv, (uint32_t, bool, uint32_t), ());
  MOCK_METHOD(bool, writeCv, (uint32_t, bool, uint32_t), ());
  MOCK_METHOD(void,
              readCv,
              (uint32_t, uint8_t, std::function<void(uint8_t)>),
              ());
  MOCK_METHOD(void,
              writeCv,
              (uint32_t, uint8_t, std::function<void(uint8_t)>),
              ());
  MOCK_METHOD(void, transmitBiDi, (std::span<uint8_t const>), ());
  MOCK_METHOD(void, eastWestDirection, (uint16_t, std::optional<int32_t>), ());
};
