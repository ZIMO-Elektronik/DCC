#include "rx_test.hpp"

TEST_F(RxTest, consist_control) {
  BASIC_ADDRESS_EXPECT_CALL_READ_CV_INIT_SEQUENCE();
  auto cv19{RandomInterval<uint8_t>(0u, 255u)};
  auto packet{make_set_consist_address_packet(_addrs.primary, cv19)};

  EXPECT_CALL(_mock,
              writeCv(Matcher<uint32_t>(19u - 1u),
                      Matcher<uint8_t>(cv19),
                      Matcher<std::function<void(uint8_t)>>(_)))
    .WillOnce(InvokeArgument<2uz>(cv19));
  if constexpr (DCC_STANDARD_COMPLIANCE) ReceiveAndExecute(packet);
  else ReceiveAndExecuteTwice(packet);
}

TEST_F(RxTest, consist_control_wrong_packet_length) {
  auto cv19{RandomInterval<uint8_t>(0u, 255u)};
  auto packet{TinkerWithPacketLength(
    make_set_consist_address_packet(_addrs.primary, cv19))};

  EXPECT_CALL(_mock,
              writeCv(Matcher<uint32_t>(19u - 1u),
                      Matcher<uint8_t>(cv19),
                      Matcher<std::function<void(uint8_t)>>(_)))
    .Times(0);
  if constexpr (DCC_STANDARD_COMPLIANCE) ReceiveAndExecute(packet);
  else ReceiveAndExecuteTwice(packet);
}

// https://github.com/ZIMO-Elektronik/DCC/issues/82
TEST_F(RxTest, consist_control_wrong_encoding) {
  dcc::Packet packet{static_cast<uint8_t>(_addrs.primary), 0x16u, 0x15u, 0x00u};

  EXPECT_CALL(_mock,
              writeCv(Matcher<uint32_t>(19u - 1u),
                      Matcher<uint8_t>(_),
                      Matcher<std::function<void(uint8_t)>>(_)))
    .Times(0);
  if constexpr (DCC_STANDARD_COMPLIANCE) ReceiveAndExecute(packet);
  else ReceiveAndExecuteTwice(packet);
}
