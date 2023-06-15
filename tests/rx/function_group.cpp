#include "rx_test.hpp"

TEST_F(RxTest, function_group_f4_f0) {
  auto state{RandomInterval(0b0'0000u, 0b1'1111u)};
  Expectation function{EXPECT_CALL(mock_, function(3u, 0b11111u, state))};
  Receive(dcc::make_function_group_f4_f0_packet(3u, state));
  Execute();
}

// Ignore F0 if CV29:2 is 0
TEST_F(RxTest, function_group_f4_f0_exception) {
  cvs_[29uz - 1uz] = 0b1000u;
  SetUp();
  Expectation f{EXPECT_CALL(mock_, function(3u, 0b11110u, 0b1u))};
  Receive(dcc::make_function_group_f4_f0_packet(3u, 0b1u));
  Execute();
}

TEST_F(RxTest, function_group_f8_f5) {
  auto state{RandomInterval(0x0u, 0xFu)};
  Expectation function{
    EXPECT_CALL(mock_, function(3u, 0xFu << 5u, state << 5u))};
  Receive(dcc::make_function_group_f8_f5_packet(3u, state));
  Execute();
}

TEST_F(RxTest, function_group_f12_f9) {
  auto state{RandomInterval(0x0u, 0xFu)};
  Expectation function{
    EXPECT_CALL(mock_, function(3u, 0xFu << 9u, state << 9u))};
  Receive(dcc::make_function_group_f12_f9_packet(3u, state));
  Execute();
}