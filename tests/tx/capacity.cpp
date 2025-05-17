#include "tx_test.hpp"

TEST_F(TxTest, capacity) { EXPECT_EQ(_mock.capacity(), DCC_TX_DEQUE_SIZE); }
