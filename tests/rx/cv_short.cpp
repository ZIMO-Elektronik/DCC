#include "rx_test.hpp"

TEST_F(RxTest, cv_short_cv23) {
  auto cv23{RandomInterval<uint8_t>(0u, 255u)};
  auto packet{
    dcc::make_cv_access_short_write_packet(_addrs.primary, 0b0010u, cv23)};

  EXPECT_CALL(_mock, writeCv(23u - 1u, cv23));
  ReceiveAndExecute(packet);
}

TEST_F(RxTest, cv_short_cv31_32) {
  uint8_t cv31{145u};
  uint8_t cv32{0u};
  auto packet{dcc::make_cv_access_short_write_packet(
    _addrs.primary, 0b0101u, cv31, cv32)};

  EXPECT_CALL(_mock, writeCv(31u - 1u, cv31));
  EXPECT_CALL(_mock, writeCv(32u - 1u, cv32));
  ReceiveAndExecuteTwice(packet);
}
