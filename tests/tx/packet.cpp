#include "tx_test.hpp"

TEST_F(TxTest, packet) {
  auto packet{dcc::make_idle_packet()};
  EXPECT_TRUE(_packet_mock.packet(packet));
  EXPECT_TRUE(_timings_mock.packet(packet));
}
