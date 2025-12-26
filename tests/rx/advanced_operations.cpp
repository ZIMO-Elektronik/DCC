#include "rx_test.hpp"

// Speed, direction and functions F7-F0
TEST_F(RxTest, speed_direction_and_functions_f7_f0) {
  EXPECT_CALL(_mock, direction(_addrs.primary.value, dcc::Forward));
  EXPECT_CALL(_mock,
              speed(_addrs.primary.value, dcc::scale_speed<126>(10 - 1)));
  EXPECT_CALL(_mock, function(_addrs.primary.value, 0xFFu, 0b0000'1010u));
  ReceiveAndExecute(make_speed_direction_and_functions_packet(
    _addrs.primary, dcc::Forward << 7u | 10u, 0b0000'1010u));
}

// Speed, direction and functions F23-F0
TEST_F(RxTest, speed_direction_and_functions_f23_f0) {
  EXPECT_CALL(_mock, direction(_addrs.primary.value, dcc::Forward));
  EXPECT_CALL(_mock,
              speed(_addrs.primary.value, dcc::scale_speed<126>(10 - 1)));
  EXPECT_CALL(_mock, function(_addrs.primary.value, 0xFFu << 0u, 1u << 0u));
  EXPECT_CALL(_mock, function(_addrs.primary.value, 0xFFu << 8u, 2u << 8u));
  EXPECT_CALL(_mock, function(_addrs.primary.value, 0xFFu << 16u, 3u << 16u));
  ReceiveAndExecute(make_speed_direction_and_functions_packet(
    _addrs.primary, dcc::Forward << 7u | 10u, 1u, 2u, 3u));
}

// 126 speed steps command forward
TEST_F(RxTest, _126_speed_steps) {
  EXPECT_CALL(_mock, direction(_addrs.primary.value, dcc::Forward));
  EXPECT_CALL(_mock, speed(_addrs.primary.value, dcc::scale_speed<126>(81)));
  ReceiveAndExecute(make_128_speed_step_control_packet(
    _addrs.primary, dcc::Forward << 7u | 0b0101'0010u));

  EXPECT_CALL(_mock, direction(_addrs.primary.value, dcc::Backward));
  EXPECT_CALL(_mock, speed(_addrs.primary.value, dcc::scale_speed<126>(23)));
  ReceiveAndExecute(make_128_speed_step_control_packet(
    _addrs.primary, dcc::Backward << 7u | 0b0001'1000u));
}

// 126 speed steps command forward
TEST_F(RxTest, _126_speed_steps_fwd_wrong_packet_length) {
  EXPECT_CALL(_mock, direction(_addrs.primary.value, dcc::Forward)).Times(0);
  EXPECT_CALL(_mock, speed(_addrs.primary.value, dcc::scale_speed<126>(10 - 1)))
    .Times(0);
  ReceiveAndExecute(TinkerWithPacketLength(make_128_speed_step_control_packet(
    _addrs.primary, dcc::Forward << 7u | 10u)));
}
