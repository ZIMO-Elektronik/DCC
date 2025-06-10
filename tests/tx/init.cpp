#include "tx_test.hpp"

TEST_F(TxTest, init) {
  dcc::tx::Config cfg{
    .num_preamble = 1u, .bit1_duration = 51u, .bit0_duration = 113u};
  ASSERT_DEBUG_DEATH(_packet_mock.init(cfg), ".*");
  ASSERT_DEBUG_DEATH(_timings_mock.init(cfg), ".*");
}
