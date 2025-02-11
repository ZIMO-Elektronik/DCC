#include "rx_test.hpp"

TEST_F(RxTest, ignore_write_bit_operations_mode) {
  // Don't write any CV which might trigger config (e.g. 1, 28, ...)!
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto bit{RandomInterval(0u, 1u)};
  auto position{RandomInterval(0u, 7u)};
  auto packet{dcc::make_cv_access_long_write_packet(
    _addrs.primary, cv_addr, bit, position)};

  // 2 or more identical packets
  EXPECT_CALL(_mock, writeCv(cv_addr, bit, position)).Times(0);
  for (auto i{0uz}; i < 2uz; ++i) {
    Receive(packet);
    Execute();
  }
}

TEST_F(RxTest, write_bit_service_mode) {
  EnterServiceMode();

  // Don't write any CV which might trigger config (e.g. 1, 28, ...)!
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto bit{RandomInterval(0u, 1u)};
  auto position{RandomInterval(0u, 7u)};
  auto packet{
    dcc::make_cv_access_long_write_service_packet(cv_addr, bit, position)};

  // 5 or more identical packets
  EXPECT_CALL(_mock, writeCv(cv_addr, bit, position)).WillOnce(Return(bit));
  EXPECT_CALL(_mock, serviceAck());
  for (auto i{0uz}; i < 5uz; ++i) {
    Receive(packet);
    Execute();
  }
}