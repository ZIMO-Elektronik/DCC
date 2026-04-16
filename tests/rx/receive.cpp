#include "rx_test.hpp"

TEST_F(RxTest, invalid_bit_resets_internal_state_machine) {
  auto state{RandomInterval<uint8_t>(0b0'0000u, 0b1'1111u)};
  EXPECT_CALL(_mock, function(_addrs.primary.value, 0b11111u, state)).Times(0);

  // Sprinkle invalid timings into the packet
  for (auto timings{
         dcc::tx::packet2timings(make_f0_f4_packet(_addrs.primary, state))};
       auto t : timings) {
    _mock.receive(t);
    _mock.receive(RandomInterval<uint32_t>(
      dcc::rx::Timing::Bit0MaxAnalog, std::numeric_limits<uint32_t>::max()));
    _mock.execute();
  }

  Execute();
}

TEST_F(RxTest, minimum_number_of_preamble_bits) {
  EXPECT_CALL(_mock, direction(_addrs.primary.value, dcc::Forward));
  EXPECT_CALL(_mock, speed(_addrs.primary.value, dcc::scale_speed<28>(17)));
  ReceiveAndExecute(
    make_speed_and_direction_packet(_addrs.primary, 1u << 5u | 0b1010u),
    {.num_preamble = DCC_RX_MIN_PREAMBLE_BITS});
}

TEST_F(RxTest, preamble_bits_can_be_part_of_invalid_packets) {
  // Last 9 bits of this packet are already part of a new preamble
  dcc::Packet invalid_idle_packet{0b1111'1111u, 0b0000'1000u, 0b1111'1111u};
  Receive(invalid_idle_packet);

  // Make timings for speed and direction packet with minimum number of preamble
  auto timings{dcc::tx::packet2timings(
    make_speed_and_direction_packet(_addrs.primary, 1u << 5u | 0b1010u),
    {.num_preamble = DCC_RX_MIN_PREAMBLE_BITS})};

  // Only transmit a needed additional preamble bits
  for (auto i{(DCC_RX_MIN_PREAMBLE_BITS - 1u) * 2uz}; i < size(timings); ++i)
    _mock.receive(timings[static_cast<dcc::tx::Timings::size_type>(i)]);

  // Packet should still be received correctly
  EXPECT_CALL(_mock, direction(_addrs.primary.value, dcc::Forward));
  EXPECT_CALL(_mock, speed(_addrs.primary.value, dcc::scale_speed<28>(17)));
  LeaveCutout()->Execute();
}
