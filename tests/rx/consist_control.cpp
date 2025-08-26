#include "rx_test.hpp"

TEST_F(RxTest, consist_control) {
  EXPECT_CALL(_mock, readCv(_))
    .WillOnce(Return(_cvs[29uz - 1uz]))
    .WillOnce(Return(_cvs[1uz - 1uz]))
    .WillOnce(Return(_cvs[19uz - 1uz]))
    .WillOnce(Return(_cvs[20uz - 1uz]))
    .WillOnce(Return(_cvs[15uz - 1uz]))
    .WillOnce(Return(_cvs[16uz - 1uz]))
    .WillOnce(Return(_cvs[28uz - 1uz]))
    .WillOnce(Return(_cvs[250uz - 1uz]))
    .WillOnce(Return(_cvs[251uz - 1uz]))
    .WillOnce(Return(_cvs[252uz - 1uz]))
    .WillOnce(Return(_cvs[253uz - 1uz]))
    .WillOnce(Return(_cvs[65297uz - 1uz]))
    .WillOnce(Return(_cvs[65298uz - 1uz]))
    .WillOnce(Return(_cvs[65299uz - 1uz]));
  auto cv19{RandomInterval<uint8_t>(0u, 255u)};
  auto packet{make_consist_control_packet(_addrs.primary, cv19)};

  EXPECT_CALL(_mock, writeCv(19u - 1u, cv19));
  if constexpr (DCC_STANDARD_COMPLIANCE) ReceiveAndExecute(packet);
  else ReceiveAndExecuteTwice(packet);
}

TEST_F(RxTest, consist_control_wrong_packet_length) {
  auto cv19{RandomInterval<uint8_t>(0u, 255u)};
  auto packet{
    TinkerWithPacketLength(make_consist_control_packet(_addrs.primary, cv19))};

  EXPECT_CALL(_mock, writeCv(19u - 1u, cv19)).Times(0);
  if constexpr (DCC_STANDARD_COMPLIANCE) ReceiveAndExecute(packet);
  else ReceiveAndExecuteTwice(packet);
}

// https://github.com/ZIMO-Elektronik/DCC/issues/82
TEST_F(RxTest, consist_control_wrong_encoding) {
  dcc::Packet packet{static_cast<uint8_t>(_addrs.primary), 0x16u, 0x15u, 0x00u};

  EXPECT_CALL(_mock, writeCv(19u - 1u, _)).Times(0);
  if constexpr (DCC_STANDARD_COMPLIANCE) ReceiveAndExecute(packet);
  else ReceiveAndExecuteTwice(packet);
}
