#include "rx_test.hpp"

TEST_F(RxTest, verify_byte_operations_mode) {
  auto cv_addr{RandomInterval(0u, smath::pow(2u, 10u) - 1u)};
  Expectation read_cv{EXPECT_CALL(_mock, readCv(cv_addr, 0u))};
  auto packet{dcc::make_cv_access_long_verify_packet(3u, cv_addr)};
  Receive(packet);
  Execute();
}

TEST_F(RxTest, verify_byte_service_mode) {
  EnterServiceMode();

  auto cv_addr{RandomInterval(0u, smath::pow(2u, 10u) - 1u)};
  Expectation read_cv{
    EXPECT_CALL(_mock, readCv(cv_addr, 42u)).WillOnce(Return(42u))};
  Expectation ack{EXPECT_CALL(_mock, serviceAck())};
  auto packet{dcc::make_cv_access_long_verify_service_packet(cv_addr, 42u)};

  // 5 or more identical packets
  for (auto i{0uz}; i < 5uz; ++i) {
    Receive(packet);
    Execute();
  }
}