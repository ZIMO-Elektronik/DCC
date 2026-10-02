#include "rx_test.hpp"

TEST_F(RxTest, read_data_space_0) {
  Logon();

  // Confirm via 8x ACK
  InSequence s;
  EXPECT_CALL(_mock,
              transmitBiDi(DatagramMatcher(
                std::array{dcc::bidi::acks[0uz], dcc::bidi::acks[0uz]})));
  EXPECT_CALL(_mock,
              transmitBiDi(DatagramMatcher(std::array{dcc::bidi::acks[0uz],
                                                      dcc::bidi::acks[0uz],
                                                      dcc::bidi::acks[0uz],
                                                      dcc::bidi::acks[0uz],
                                                      dcc::bidi::acks[0uz],
                                                      dcc::bidi::acks[0uz]})));
  Receive(
    dcc::make_logon_select_packet(DCC_MANUFACTURER_ID, _did, 0b1111'1110u, 0u));
  BiDi();

  // Read data space 0
  EXPECT_CALL(_mock, readCv(_))
    .Times(31)
    .WillRepeatedly([count = 0u](uint32_t) mutable { return count++; });
  LeaveCutout()->Execute();

  // Transmit it within 6 cutouts
}
