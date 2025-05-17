#include "rx_test.hpp"

TEST_F(RxTest, function_group_f4_f0) {
  auto state{RandomInterval<uint8_t>(0b0'0000u, 0b1'1111u)};
  EXPECT_CALL(_mock, function(_addrs.primary.value, 0b11111u, state));
  Receive(dcc::make_function_group_f4_f0_packet(_addrs.primary, state));
  Execute();
}

// Ignore F0 if CV29:2 is 0
TEST_F(RxTest, function_group_f4_f0_exception) {
  _cvs[29uz - 1uz] = 0b1000u;
  SetUp();
  EXPECT_CALL(_mock, function(_addrs.primary.value, 0b11110u, 0b1u));
  Receive(dcc::make_function_group_f4_f0_packet(_addrs.primary, 0b1u));
  Execute();
}

TEST_F(RxTest, function_group_f8_f5) {
  auto state{RandomInterval<uint8_t>(0x0u, 0xFu)};
  EXPECT_CALL(_mock,
              function(_addrs.primary.value,
                       0xFu << 5u,
                       static_cast<uint32_t>(state << 5u)));
  Receive(dcc::make_function_group_f8_f5_packet(_addrs.primary, state));
  Execute();
}

TEST_F(RxTest, function_group_f12_f9) {
  auto state{RandomInterval<uint8_t>(0x0u, 0xFu)};
  EXPECT_CALL(_mock,
              function(_addrs.primary.value,
                       0xFu << 9u,
                       static_cast<uint32_t>(state << 9u)));
  Receive(dcc::make_function_group_f12_f9_packet(_addrs.primary, state));
  Execute();
}
