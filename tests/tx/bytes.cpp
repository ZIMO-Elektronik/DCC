#include "tx_test.hpp"

TEST_F(TxTest, bytes) {
  auto packet{dcc::make_idle_packet()};
  EXPECT_TRUE(_packet_mock.bytes(packet));
  EXPECT_TRUE(_timings_mock.bytes(packet));
}
