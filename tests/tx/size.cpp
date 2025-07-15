#include "tx_test.hpp"

TEST_F(TxTest, initial_size) {
  EXPECT_ALL_EQ(0uz, _packet_mock.size(), _timings_mock.size());
}

TEST_F(TxTest, size) {
  _packet_mock.packet(dcc::make_idle_packet());
  _timings_mock.packet(dcc::make_idle_packet());
  EXPECT_ALL_EQ(1uz, _packet_mock.size(), _timings_mock.size());
}
