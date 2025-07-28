#include "rx_test.hpp"

TEST_F(RxTest, cv_short_cv23) {
  auto cv23{RandomInterval<uint8_t>(0u, 255u)};

  EXPECT_CALL(_mock, writeCv(23u - 1u, cv23));
  if constexpr (DCC_STANDARD_COMPLIANCE) {
    Receive(
      dcc::make_cv_access_short_write_packet(_addrs.primary, 0b0010u, cv23));
    Execute();
  } else
    ReceiveAndExecuteTwoIdenticalCvShortWritePackets(
      _addrs.primary, 0b0010u, cv23);
}

TEST_F(RxTest, cv_short_cv31_32) {
  uint8_t cv31{145u};
  uint8_t cv32{0u};

  EXPECT_CALL(_mock, writeCv(31u - 1u, cv31));
  EXPECT_CALL(_mock, writeCv(32u - 1u, cv32));
  ReceiveAndExecuteTwoIdenticalCvShortWritePackets(
    _addrs.primary, 0b0101u, cv31, cv32);
  Execute();
}
