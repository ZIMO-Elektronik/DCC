#include "rx_test.hpp"

TEST_F(RxTest, invalid_bit_resets_internal_state_machine) {
  auto state{RandomInterval<uint8_t>(0b0'0000u, 0b1'1111u)};
  EXPECT_CALL(_mock, function(_addrs.primary.value, 0b11111u, state)).Times(0);

  // Springe invalid timings into the packet
  for (auto timings{dcc::tx::packet2timings(
         dcc::make_function_group_f4_f0_packet(_addrs.primary, state))};
       auto t : timings) {
    _mock.receive(t);
    _mock.receive(RandomInterval<uint32_t>(
      dcc::rx::Timing::Bit0MaxAnalog, std::numeric_limits<uint32_t>::max()));
    _mock.execute();
  }

  Execute();
}
