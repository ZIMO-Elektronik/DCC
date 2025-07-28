#include "rx_test.hpp"

TEST_F(RxTest, speed_and_direction) {
  EXPECT_CALL(_mock, direction(_addrs.primary.value, dcc::Forward));
  EXPECT_CALL(_mock, speed(_addrs.primary.value, dcc::scale_speed<28>(17)));
  ReceiveAndExecute(
    make_speed_and_direction_packet(_addrs.primary, 1u << 5u | 10u));
}

TEST_F(RxTest, speed_and_direction_wrong_packet_length) {
  EXPECT_CALL(_mock, direction(_addrs.primary.value, dcc::Forward)).Times(0);
  EXPECT_CALL(_mock, speed(_addrs.primary.value, dcc::scale_speed<28>(17)))
    .Times(0);
  ReceiveAndExecute(TinkerWithPacketLength(
    make_speed_and_direction_packet(_addrs.primary, 1u << 5u | 10u)));
}
