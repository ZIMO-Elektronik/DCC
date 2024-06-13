#include "rx_test.hpp"

TEST_F(RxTest, write_byte_operations_mode) {
  // Don't write any CV which might trigger config (e.g. 1, 28, ...)!
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto cv_value{RandomInterval<uint8_t>(0u, 255u)};

  Expectation write_cv{EXPECT_CALL(_mock, writeCv(cv_addr, cv_value))};
  ReceiveAndExecuteTwoIdenticalCvWritePackets(cv_addr, cv_value);
}

TEST_F(RxTest, write_byte_operations_mode_requires_two_identical_packets) {
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto cv_value{RandomInterval<uint8_t>(0u, 255u)};
  auto packet{
    dcc::make_cv_access_long_write_packet(_addrs.primary, cv_addr, cv_value)};

  Expectation do_not_write_cv{
    EXPECT_CALL(_mock, writeCv(cv_addr, cv_value)).Times(0)};
  Receive(packet);
  Execute();
}

TEST_F(RxTest, write_byte_service_mode) {
  EnterServiceMode();

  // Don't write any CV which might trigger config (e.g. 1, 28, ...)!
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto cv_value{RandomInterval<uint8_t>(0u, 255u)};
  auto packet{dcc::make_cv_access_long_write_service_packet(cv_addr, cv_value)};

  // 5 or more identical packets
  Expectation write_cv{EXPECT_CALL(_mock, writeCv(cv_addr, cv_value))};
  for (auto i{0uz}; i < 5uz; ++i) {
    Receive(packet);
    Execute();
  }
}