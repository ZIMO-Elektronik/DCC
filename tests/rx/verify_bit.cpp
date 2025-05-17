#include "rx_test.hpp"

TEST_F(RxTest, verify_bit_service_mode) {
  EnterServiceMode();

  // Don't write any CV which might trigger config (e.g. 1, 28, ...)!
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto bit{RandomInterval(0u, 1u)};
  auto position{RandomInterval(0u, 7u)};
  auto packet{
    dcc::make_cv_access_long_verify_service_packet(cv_addr, bit, position)};

  // 5 or more identical packets
  EXPECT_CALL(_mock, readCv(cv_addr, bit, position)).WillOnce(Return(bit));
  EXPECT_CALL(_mock, serviceAck());
  for (auto i{0uz}; i < 5uz; ++i) {
    Receive(packet);
    Execute();
  }
}
