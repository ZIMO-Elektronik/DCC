#include "rx_test.hpp"

TEST_F(RxTest, cv15_not_equal_cv15_activates_decoder_lock) {
  _cvs[15uz - 1uz] = 1u;
  _cvs[16uz - 1uz] = 2u;
  SetUp();
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto cv_value{RandomInterval(0u, 255u)};
  Expectation can_not_write_cv{
    EXPECT_CALL(_mock, writeCv(cv_addr, cv_value)).Times(0)};
  ReceiveAndExecuteTwoIdenticalCvWritePackets(cv_addr, cv_value);
}

TEST_F(RxTest, cv15_zero_deactivates_decoder_lock) {
  _cvs[15uz - 1uz] = 0u;
  _cvs[16uz - 1uz] = 1u;
  SetUp();
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto cv_value{RandomInterval(0u, 255u)};
  Expectation write_cv{EXPECT_CALL(_mock, writeCv(cv_addr, cv_value))};
  ReceiveAndExecuteTwoIdenticalCvWritePackets(cv_addr, cv_value);
}

TEST_F(RxTest, cv16_zero_deactivates_decoder_lock) {
  _cvs[15uz - 1uz] = 42u;
  _cvs[16uz - 1uz] = 0u;
  SetUp();
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto cv_value{RandomInterval(0u, 255u)};
  Expectation write_cv{EXPECT_CALL(_mock, writeCv(cv_addr, cv_value))};
  ReceiveAndExecuteTwoIdenticalCvWritePackets(cv_addr, cv_value);
}