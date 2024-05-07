#include "rx_test.hpp"

// 126 speed steps command forward
TEST_F(RxTest, _126_speed_steps_fwd) {
  Expectation dir{EXPECT_CALL(_mock, direction(3u, dcc::Forward))};
  Expectation speed{
    EXPECT_CALL(_mock, speed(3u, dcc::scale_speed<126>(10 - 1)))};
  Receive(dcc::make_advanced_operations_speed_packet(3u, 1u << 7u | 10u));
  Execute();
}

// 126 speed steps command backward
TEST_F(RxTest, _126_speed_steps_bwd) {
  Expectation dir{EXPECT_CALL(_mock, direction(3u, dcc::Backward))};
  Expectation speed{EXPECT_CALL(_mock, speed(3u, _))};
  Receive(dcc::make_advanced_operations_speed_packet(3u, 0u << 7u | 10u));
  Execute();
}
