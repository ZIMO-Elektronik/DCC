#include "rx_test.hpp"

// Speed and direction
TEST_F(RxTest, speed_and_direction) {
  EXPECT_CALL(_mock, direction(_addrs.primary.value, dcc::Forward));
  EXPECT_CALL(_mock, speed(_addrs.primary.value, dcc::scale_speed<28>(17)));
  Receive(dcc::make_speed_and_direction_packet(_addrs.primary, 1u << 5u | 10u));
  Execute();
}