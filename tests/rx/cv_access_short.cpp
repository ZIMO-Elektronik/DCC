#include "rx_test.hpp"

TEST_F(RxTest, cv_access_short_cv17_18) {
  dcc::Address new_extended_address{.value = 1337u,
                                    .type = dcc::Address::ExtendedLoco};
  encode_address(new_extended_address, &_cvs[17uz - 1uz]);

  // Before switching basic address init sequence is invoked twice
  EXPECT_CALL(_mock, readCv(_))
    .BASIC_ADDRESS_READ_CV_INIT_SEQUENCE()
    .BASIC_ADDRESS_READ_CV_INIT_SEQUENCE()
    .EXTENDED_ADDRESS_READ_CV_INIT_SEQUENCE_CHANGE_CV29(_cvs[29uz - 1uz] |
                                                        ztl::mask<5u>);

  // Change address
  for (auto i{0uz}; i < 2uz; ++i)
    ReceiveAndExecute(make_cv_access_short_write_packet(
      _addrs.primary, 0b0100u, _cvs[17uz - 1uz], _cvs[18uz - 1uz]));

  // Execute commands to address 1337
  auto state{RandomInterval<uint8_t>(0b0'0000u, 0b1'1111u)};
  EXPECT_CALL(_mock, function(new_extended_address.value, 0b11111u, state));
  ReceiveAndExecute(make_f0_f4_packet(new_extended_address, state));
}

TEST_F(RxTest, cv_access_short_cv23) {
  auto cv23{RandomInterval<uint8_t>(0u, 255u)};
  auto packet{
    dcc::make_cv_access_short_write_packet(_addrs.primary, 0b0010u, cv23)};

  EXPECT_CALL(_mock,
              writeCv(Matcher<uint32_t>(23u - 1u),
                      Matcher<uint8_t>(cv23),
                      Matcher<std::function<void(uint8_t)>>(_)))
    .WillOnce(InvokeArgument<2uz>(cv23));
  if constexpr (DCC_STANDARD_COMPLIANCE) ReceiveAndExecute(packet);
  else ReceiveAndExecuteTwice(packet);
}

TEST_F(RxTest, cv_access_short_cv31_32) {
  uint8_t cv31{145u};
  uint8_t cv32{0u};
  auto packet{dcc::make_cv_access_short_write_packet(
    _addrs.primary, 0b0101u, cv31, cv32)};

  EXPECT_CALL(_mock,
              writeCv(Matcher<uint32_t>(31u - 1u),
                      Matcher<uint8_t>(cv31),
                      Matcher<std::function<void(uint8_t)>>(_)))
    .WillOnce(InvokeArgument<2uz>(cv31));
  EXPECT_CALL(_mock,
              writeCv(Matcher<uint32_t>(32u - 1u),
                      Matcher<uint8_t>(cv32),
                      Matcher<std::function<void(uint8_t)>>(_)))
    .WillOnce(InvokeArgument<2uz>(cv32));
  ReceiveAndExecuteTwice(packet);
}
