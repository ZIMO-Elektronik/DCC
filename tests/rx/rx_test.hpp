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

  RxTest* Receive(dcc::Packet const& packet);
  RxTest* BiDiChannel1();
  RxTest* BiDiChannel2();
  RxTest* BiDi();
  RxTest* LeaveCutout();
  RxTest* Execute();

  void EnterServiceMode();

  void Logon();

  dcc::Packet TinkerWithPacketLength(dcc::Packet packet) const;

  template<std::unsigned_integral T>
  static T RandomInterval(T min, T max) {
    std::mt19937 gen{std::random_device{}()};
    std::uniform_int_distribution<T> dis{min, max};
    return dis(gen);
  }

  /// \todo this needs to go
  void ReceiveAndExecute(dcc::Packet const& packet) {
    Receive(packet)->LeaveCutout()->Execute();
  }

  /// \todo this needs to go
  void ReceiveAndExecuteTwice(dcc::Packet const& packet) {
    ReceiveAndExecute(packet);
    ReceiveAndExecute(packet);
  }

  NiceMock<RxMock> _mock;
  dcc::Addresses _addrs{
    .primary = {.value = 3u, .type = dcc::Address::BasicLoco},
    .consist = {.value = 4u, .type = dcc::Address::BasicLoco},
    .logon = {.value = 1000u, .type = dcc::Address::ExtendedLoco}};
  std::array<uint8_t, smath::pow(2uz, 16uz)> _cvs{};
  uint32_t _did{0xAABBCCDDu};
  uint16_t _cid{0xABCDu};
  uint8_t _sid{0x2Au};
};

MATCHER_P(DatagramMatcher, datagram, "") {
  return std::equal(cbegin(datagram), cend(datagram), cbegin(arg));
}

#define BASIC_ADDRESS_EXPECT_CALL_READ_CV_INIT_SEQUENCE()                      \
  EXPECT_CALL(_mock, readCv(_))                                                \
    .WillOnce(Return(_cvs[29uz - 1uz]))                                        \
    .WillOnce(Return(_cvs[1uz - 1uz]))                                         \
    .WillOnce(Return(_cvs[19uz - 1uz]))                                        \
    .WillOnce(Return(_cvs[20uz - 1uz]))                                        \
    .WillOnce(Return(_cvs[15uz - 1uz]))                                        \
    .WillOnce(Return(_cvs[16uz - 1uz]))                                        \
    .WillOnce(Return(_cvs[28uz - 1uz]))                                        \
    .WillOnce(Return(_cvs[DCC_RX_LOGON_DID_CV_ADDRESS + 0uz]))                 \
    .WillOnce(Return(_cvs[DCC_RX_LOGON_DID_CV_ADDRESS + 1uz]))                 \
    .WillOnce(Return(_cvs[DCC_RX_LOGON_DID_CV_ADDRESS + 2uz]))                 \
    .WillOnce(Return(_cvs[DCC_RX_LOGON_DID_CV_ADDRESS + 3uz]))                 \
    .WillOnce(Return(_cvs[DCC_RX_LOGON_CID_CV_ADDRESS + 0uz]))                 \
    .WillOnce(Return(_cvs[DCC_RX_LOGON_CID_CV_ADDRESS + 1uz]))                 \
    .WillOnce(Return(_cvs[DCC_RX_LOGON_SID_CV_ADDRESS]))                       \
    .WillOnce(Return(_cvs[DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 0u]))              \
    .WillOnce(Return(_cvs[DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 1u]))

#define EXTENDED_ADDRESS_EXPECT_CALL_READ_CV_INIT_SEQUENCE()                   \
  EXPECT_CALL(_mock, readCv(_))                                                \
    .WillOnce(Return(_cvs[29uz - 1uz]))                                        \
    .WillOnce(Return(_cvs[17uz - 1uz]))                                        \
    .WillOnce(Return(_cvs[18uz - 1uz]))                                        \
    .WillOnce(Return(_cvs[19uz - 1uz]))                                        \
    .WillOnce(Return(_cvs[20uz - 1uz]))                                        \
    .WillOnce(Return(_cvs[15uz - 1uz]))                                        \
    .WillOnce(Return(_cvs[16uz - 1uz]))                                        \
    .WillOnce(Return(_cvs[28uz - 1uz]))                                        \
    .WillOnce(Return(_cvs[DCC_RX_LOGON_DID_CV_ADDRESS + 0uz]))                 \
    .WillOnce(Return(_cvs[DCC_RX_LOGON_DID_CV_ADDRESS + 1uz]))                 \
    .WillOnce(Return(_cvs[DCC_RX_LOGON_DID_CV_ADDRESS + 2uz]))                 \
    .WillOnce(Return(_cvs[DCC_RX_LOGON_DID_CV_ADDRESS + 3uz]))                 \
    .WillOnce(Return(_cvs[DCC_RX_LOGON_CID_CV_ADDRESS + 0uz]))                 \
    .WillOnce(Return(_cvs[DCC_RX_LOGON_CID_CV_ADDRESS + 1uz]))                 \
    .WillOnce(Return(_cvs[DCC_RX_LOGON_SID_CV_ADDRESS]))                       \
    .WillOnce(Return(_cvs[DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 0u]))              \
    .WillOnce(Return(_cvs[DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 1u]))
