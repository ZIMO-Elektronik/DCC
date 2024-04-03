#pragma once

#include <gtest/gtest.h>
#include <random>
#include "rx_mock.hpp"

using namespace ::testing;

// Receive test fixture
struct RxTest : ::testing::Test {
protected:
  RxTest();
  virtual ~RxTest();

  void SetUp() override;

  void Receive(dcc::Packet const& packet);
  void Receive(dcc::tx::Timings const& timings);
  void Execute();
  void Cutout();

  void EnterServiceMode();

  void ReceiveAndExecuteTwoIdenticalCvWritePackets(uint32_t addr, uint8_t byte);

  template<std::unsigned_integral T>
  static T RandomInterval(T min, T max) {
    std::mt19937 gen{std::random_device{}()};
    std::uniform_int_distribution<T> dis{min, max};
    return dis(gen);
  }

  RxMock _mock;
  std::array<uint8_t, smath::pow(2uz, 16uz)> _cvs{};
  uint32_t _did{0xAABBCCDDu};
  uint16_t _cid{0xABCDu};
  uint8_t _session_id{0x2Au};
};