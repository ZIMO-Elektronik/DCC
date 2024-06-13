#include "rx_test.hpp"

// 126 speed steps command forward
TEST_F(RxTest, _126_speed_steps_fwd) {
  EXPECT_CALL(_mock, direction(_addrs.primary.value, dcc::Forward));
  EXPECT_CALL(_mock,
              speed(_addrs.primary.value, dcc::scale_speed<126>(10 - 1)));
  Receive(
    dcc::make_advanced_operations_speed_packet(_addrs.primary, 1u << 7u | 10u));
  Execute();
}

// 126 speed steps command backward
TEST_F(RxTest, _126_speed_steps_bwd) {
  EXPECT_CALL(_mock, direction(_addrs.primary.value, dcc::Backward));
  EXPECT_CALL(_mock, speed(_addrs.primary.value, _));
  Receive(
    dcc::make_advanced_operations_speed_packet(_addrs.primary, 0u << 7u | 10u));
  Execute();
}
