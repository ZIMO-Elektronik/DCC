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

TEST_F(RxTest, writing_cv1_clears_cv29_5) {
  // Start with extended address
  _addrs.primary = {.value = 4321u, .type = dcc::Address::ExtendedLoco};
  encode_address(_addrs.primary, &_cvs[17uz - 1uz]);
  _cvs[29uz - 1uz] = _cvs[29uz - 1uz] | ztl::mask<5u>;
  SetUp();

  EXPECT_CALL(_mock, readCv(_))
    .BASIC_ADDRESS_READ_CV_INIT_SEQUENCE_CHANGE_CV29(
      static_cast<uint8_t>(_cvs[29uz - 1uz] & ~ztl::mask<5u>));

  // Write CV1=3 (which clears CV29:5)
  uint8_t cv_addr{1u - 1u};
  uint8_t byte{3u};
  EXPECT_CALL(_mock,
              writeCv(Matcher<uint32_t>(cv_addr),
                      Matcher<uint8_t>(byte),
                      Matcher<std::function<void(uint8_t)>>(_)))
    .WillOnce(InvokeArgument<2uz>(byte));
  EXPECT_CALL(_mock, writeCv(29u - 1u, false, 5u)).WillOnce(Return(false));
  ReceiveAndExecuteTwice(
    make_cv_access_long_write_packet(_addrs.primary, cv_addr, byte));
}
