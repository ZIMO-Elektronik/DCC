#include "tx_test.hpp"

TEST_F(TxTest, packet) {
  auto packet{dcc::make_idle_packet()};
  EXPECT_ALL_TRUE(_packet_mock.packet(packet), _timings_mock.packet(packet));
}
