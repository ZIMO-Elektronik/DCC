#include "tx_test.hpp"

TEST_F(TxTest, size) {
  EXPECT_EQ(_mock.size(), 0uz);
  _mock.packet({});
  EXPECT_EQ(_mock.size(), 1uz);
}