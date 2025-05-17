#include "tx_test.hpp"

TEST_F(TxTest, packet) {
  for (auto i{DCC_TX_DEQUE_SIZE}; i-- > 0uz;) EXPECT_TRUE(_mock.packet({}));
  EXPECT_FALSE(_mock.packet({}));
}
