#include "tx_test.hpp"

TEST_F(TxTest, initial_size) {
  EXPECT_EQ(_packet_mock.size(), 0uz);
  EXPECT_EQ(_timings_mock.size(), 0uz);
}

TEST_F(TxTest, size) {
  _packet_mock.packet(dcc::make_idle_packet());
  EXPECT_EQ(_packet_mock.size(), 1uz);
  _timings_mock.packet(dcc::make_idle_packet());
  EXPECT_EQ(_timings_mock.size(), 1uz);
}
