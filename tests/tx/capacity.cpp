#include "tx_test.hpp"

TEST_F(TxTest, capacity) {
  EXPECT_ALL_EQ(
    DCC_TX_DEQUE_SIZE, _packet_mock.capacity(), _timings_mock.capacity());
}
