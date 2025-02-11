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
  void BiDi();

  void EnterServiceMode();

  void ReceiveAndExecuteTwoIdenticalCvWritePackets(uint16_t addr,
                                                   uint32_t cv_addr,
                                                   uint8_t byte);

  void Logon();

  template<std::unsigned_integral T>
  static T RandomInterval(T min, T max) {
    std::mt19937 gen{std::random_device{}()};
    std::uniform_int_distribution<T> dis{min, max};
    return dis(gen);
  }

  NiceMock<RxMock> _mock;
  dcc::Addresses _addrs{
    .primary = {.value = 3u, .type = dcc::Address::BasicLoco},
    .consist = {.value = 4u, .type = dcc::Address::BasicLoco},
    .logon = {.value = 1000u, .type = dcc::Address::ExtendedLoco}};
  std::array<uint8_t, smath::pow(2uz, 16uz)> _cvs{};
  uint32_t _did{0xAABBCCDDu};
  uint16_t _cid{0xABCDu};
  uint8_t _session_id{0x2Au};
};

MATCHER_P(DatagramMatcher, datagram, "") {
  return std::equal(cbegin(datagram), cend(datagram), cbegin(arg));
}