#include "rx_test.hpp"

TEST_F(RxTest, write_byte_operations_mode) {
  // Don't write any CV which might trigger config (e.g. 1, 28, ...)!
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto byte{RandomInterval<uint8_t>(0u, 255u)};

  EXPECT_CALL(_mock, writeCv(cv_addr, byte));
  ReceiveAndExecuteTwoIdenticalCvWritePackets(_addrs.primary, cv_addr, byte);
}

TEST_F(RxTest, write_byte_operations_mode_requires_two_identical_packets) {
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto byte{RandomInterval<uint8_t>(0u, 255u)};
  auto packet{
    dcc::make_cv_access_long_write_packet(_addrs.primary, cv_addr, byte)};

  EXPECT_CALL(_mock, writeCv(cv_addr, byte)).Times(0);
  Receive(packet);
  Execute();
}

TEST_F(RxTest, write_byte_service_mode) {
  EnterServiceMode();

  // Don't write any CV which might trigger config (e.g. 1, 28, ...)!
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto byte{RandomInterval<uint8_t>(0u, 255u)};
  auto packet{dcc::make_cv_access_long_write_service_packet(cv_addr, byte)};

  // 5 or more identical packets
  EXPECT_CALL(_mock, writeCv(cv_addr, byte));
  for (auto i{0uz}; i < 5uz; ++i) {
    Receive(packet);
    Execute();
  }
}