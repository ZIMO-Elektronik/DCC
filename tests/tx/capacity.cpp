#include "tx_test.hpp"

TEST_F(TxTest, capacity) {
  EXPECT_EQ(_packet_mock.capacity(), DCC_TX_DEQUE_SIZE);
  EXPECT_EQ(_timings_mock.capacity(), DCC_TX_DEQUE_SIZE);
}
