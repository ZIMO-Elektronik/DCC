#pragma once

#include <gtest/gtest.h>
#include <random>
#include "rx_mock.hpp"

using namespace ::testing;

// Receive test fixture
struct RxTest : ::testing::Test {
  RxTest();
  virtual ~RxTest();

  void SetUp() override;

  void Receive(dcc::Packet const& packet);
  void Receive(dcc::tx::Timings const& timings);
  void Execute();

  void EnterServiceMode();

  void ReceiveAndExecuteTwoIdenticalCvWritePackets(uint32_t addr, uint8_t byte);

  template<std::unsigned_integral T>
  static T RandomInterval(T min, T max) {
    auto const r{static_cast<double>(_dis(_gen))};
    auto const r_max{static_cast<double>(std::numeric_limits<T>::max())};
    return static_cast<T>(ztl::lerp<double>(
      r, 0, r_max, static_cast<double>(min), static_cast<double>(max)));
  }

  static inline std::mt19937 _gen{std::random_device{}()};
  static inline std::uniform_int_distribution<uint32_t> _dis{
    0u, std::numeric_limits<uint32_t>::max()};

  RxMock _mock;
  std::array<uint8_t, smath::pow(2uz, 16uz)> _cvs{};
};