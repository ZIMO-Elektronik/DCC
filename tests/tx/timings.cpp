#include "dcc/dcc.hpp"
#include "tx_test.hpp"

TEST_F(TxTest, bytes2timings_preamble) {
  uint8_t byte = 0xC3u;
  auto tim = dcc::tx::bytes2timings(std::span<uint8_t>(&byte, 1u));

  uint16_t offset = 0;

  EvalPreamble(tim, offset);
}

TEST_F(TxTest, byte2timings_start_bit) {
  uint8_t byte = 0xC3u;
  auto tim = dcc::tx::bytes2timings(std::span<uint8_t>(&byte, 1u));

  uint16_t offset = 2 * DCC_TX_MIN_PREAMBLE_BITS;

  EvalStartBit(tim, offset);
}

TEST_F(TxTest, bytes2timings_byte) {
  uint8_t byte = 0xC3u;
  auto tim = dcc::tx::bytes2timings(std::span<uint8_t>(&byte, 1u));

  uint16_t offset = 2 * DCC_TX_MIN_PREAMBLE_BITS + 2;

  EvalByte(tim, byte, offset);
}
TEST_F(TxTest, bytes2timings_stop_bit) {
  uint8_t byte = 0xC3u;
  auto tim = dcc::tx::bytes2timings(std::span<uint8_t>(&byte, 1u));

  uint16_t offset = 2 * DCC_TX_MIN_PREAMBLE_BITS + 2 + 2 * 8;

  EvalStopBit(tim, offset);
}

TEST_F(TxTest, bytes2timings_timings_size) {
  uint8_t byte = 0xC3u;
  auto tim = dcc::tx::bytes2timings(std::span<uint8_t>(&byte, 1u));

  ASSERT_EQ(tim.size(), 2 * (DCC_TX_MIN_PREAMBLE_BITS + 1 + 8 + 1));
}

TEST_F(TxTest, bytes2timings_multiple_bytes) {
  std::array<uint8_t, 3> bytes = {0xC3u, 0xC3u, 0xC3u};
  auto tim = dcc::tx::bytes2timings(bytes);

  uint16_t offset = 2 * DCC_TX_MIN_PREAMBLE_BITS;

  for (uint16_t i = 0; i < 3; i++) {
    EvalStartBit(tim, offset);
    EvalByte(tim, bytes[i], offset);
  }
  EvalStopBit(tim, offset);
}

TEST_F(TxTest, bytes2timings_cfg_timings) {
  uint8_t byte = 0xC3u;
  dcc::tx::Config cfg{};
  cfg.bit0_duration = dcc::tx::Timing::Bit0Max;
  cfg.bit1_duration = dcc::tx::Timing::Bit1Max;
  auto tim = dcc::tx::bytes2timings(std::span<uint8_t>(&byte, 1u), cfg);

  uint16_t offset = 2 * DCC_TX_MIN_PREAMBLE_BITS + 2;

  EvalByte(tim, byte, offset, cfg);
}

TEST_F(TxTest, bytes2timings_cfg_preamble) {
  uint8_t byte = 0xC3u;
  dcc::tx::Config cfg{};
  cfg.num_preamble = DCC_TX_MIN_PREAMBLE_BITS + 2;
  auto tim = dcc::tx::bytes2timings(std::span<uint8_t>(&byte, 1u), cfg);

  uint16_t offset = 0;

  EvalPreamble(tim, offset, cfg);
}