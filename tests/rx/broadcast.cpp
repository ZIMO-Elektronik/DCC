#include "rx_test.hpp"

TEST_F(RxTest, broadcast_speed_and_direction) {
  EXPECT_CALL(_mock, direction(0u, _)).Times(0);
  EXPECT_CALL(_mock, speed(0u, dcc::scale_speed<28>(17)));
  ReceiveAndExecute(dcc::make_speed_and_direction_packet(0u, 1u << 5u | 10u));
}

TEST_F(RxTest, broadcast_speed_and_direction_f0_exception) {
  _cvs[29uz - 1uz] = 0b1000u;
  SetUp();

  EXPECT_CALL(_mock, function(0u, 1u, 1u)).Times(0);
  ReceiveAndExecute(dcc::make_speed_and_direction_packet(0u, 0b01'0000u));
}

TEST_F(RxTest, broadcast_speed_and_direction_estop) {
  EXPECT_CALL(_mock, direction(0u, _)).Times(0);
  EXPECT_CALL(_mock, speed(0u, dcc::EStop));
  ReceiveAndExecute(dcc::make_speed_and_direction_packet(0u, 0b00'0001u));
}
