#include "tx_test.hpp"

TEST_F(TxTest, bytes) {
  for (auto i{DCC_TX_DEQUE_SIZE}; i-- > 0uz;) EXPECT_TRUE(_mock.bytes({}));
  EXPECT_FALSE(_mock.bytes({}));
}
