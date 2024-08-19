#include "rx_test.hpp"

TEST_F(RxTest, consist_shall_not_act_on_cv_manipulation) {
  _cvs[19uz - 1uz] = static_cast<uint8_t>(_addrs.consist);
  SetUp();

  // Don't write any CV which might trigger config (e.g. 1, 28, ...)!
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto byte{RandomInterval<uint8_t>(0u, 255u)};

  EXPECT_CALL(_mock, writeCv(cv_addr, byte)).Times(0);
  ReceiveAndExecuteTwoIdenticalCvWritePackets(_addrs.consist, cv_addr, byte);
}