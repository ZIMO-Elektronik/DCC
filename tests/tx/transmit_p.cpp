#include "tx_test.hpp"

TEST_F(TxTest, transmit_p_preamble) {
  auto res = TransmitP(3u);
  dcc::tx::Timings::size_type offset = 0;
  ASSERT_TRUE(EvalPreamble(res, offset)) << "Malformed Preamble at " << offset;
}

TEST_F(TxTest, transmit_p_bytes) {
  dcc::Packet pkt{0xC3u, 0xC3u, 0xC3u};
  _mockp.packet(pkt);

  TransmitP(3u); // Skip idle
  auto res = TransmitP(3u);
  dcc::tx::Timings::size_type offset = 0;

  ASSERT_TRUE(EvalPacket(res, pkt, offset))
    << "Malformed Packet data at " << offset;
}

TEST_F(TxTest, transmit_p_bidi) {

  auto res = TransmitP(3u);
  uint16_t offset = 2 * DCC_TX_MIN_PREAMBLE_BITS // Preamble
                    + 3 * 2 * 9                  // 3 Byte
                    + 2;                         // Stop-Bit

  ASSERT_TRUE(EvalBiDi(res, offset)) << "Malformed  BiDi timings at " << offset;
}

TEST_F(TxTest, transmit_p_cfg_preamble) {
  dcc::tx::Config cfg{};
  cfg.num_preamble = 19;
  _mockp.init(cfg);

  dcc::Packet pkt{0xC3u, 0xC3u, 0xC3u};
  _mockp.packet(pkt);

  // Idle
  auto res = TransmitP(3u, cfg);

  dcc::tx::Timings::size_type offset = 0;

  ASSERT_TRUE(EvalPreamble(res, offset, cfg))
    << "Malformed  Preamble at " << offset;

  // Packet
  res = TransmitP(3u, cfg);

  offset = 0;

  ASSERT_TRUE(EvalPreamble(res, offset, cfg))
    << "Malformed  Preamble at " << offset;
}

TEST_F(TxTest, transmit_p_cfg_bytes) {
  dcc::tx::Config cfg{};
  cfg.bit0_duration = dcc::tx::Timing::Bit0Max;
  cfg.bit1_duration = dcc::tx::Timing::Bit1Max;
  _mockp.init(cfg);

  dcc::Packet pkt{0xC3u, 0xC3u, 0xC3u};
  _mockp.packet(pkt);

  // Idle
  auto res = TransmitP(3u, cfg);

  dcc::tx::Timings::size_type offset = 0;

  ASSERT_TRUE(EvalPacket(res, dcc::make_idle_packet(), offset, cfg))
    << "Malformed Timings data at " << offset;
  ;

  // Packet
  res = TransmitP(3u, cfg);

  offset = 0;

  ASSERT_TRUE(EvalPacket(res, pkt, offset, cfg))
    << "Malformed Timings data at " << offset;
  ;
}

TEST_F(TxTest, transmit_p_cfg_no_bidi) {
  dcc::tx::Config cfg{};
  cfg.flags.bidi = false;
  _mockp.init(cfg);

  auto pkt = dcc::make_idle_packet();

  // Idle
  auto res = TransmitP(3u, cfg);

  dcc::tx::Timings::size_type offset = 0;

  ASSERT_TRUE(EvalPacket(res, pkt, offset, cfg))
    << "Malformed Packet data at " << offset;

  // Packet
  res = TransmitP(3u, cfg);

  offset = 0;

  ASSERT_TRUE(EvalPacket(res, pkt, offset, cfg))
    << "Malformed second Packet data at " << offset;
}

TEST_F(TxTest, transmit_p_idle) {

  auto res = TransmitP(3u);

  auto pkt = dcc::make_idle_packet();
  dcc::tx::Timings::size_type offset = 0;

  ASSERT_TRUE(EvalPacket(res, pkt, offset))
    << "Malformed Idle Packet data at " << offset;
}

TEST_F(TxTest, transmit_p_paket_after_idle) {
  dcc::Packet pkt{0xC3u, 0xC3u, 0xC3u};
  _mockp.packet(pkt);

  auto i_pkt = dcc::make_idle_packet();

  auto res = TransmitP(3u);

  dcc::tx::Timings::size_type offset = 0;

  ASSERT_TRUE(EvalPacket(res, i_pkt, offset))
    << "Malformed Idle Packet data at " << offset;

  offset = 0;
  res = TransmitP(3u);

  ASSERT_TRUE(EvalPacket(res, pkt, offset))
    << "Malformed Packet data at " << offset;
}

TEST_F(TxTest, transmit_p_idle_after_paket) {
  dcc::Packet pkt{0xC3u, 0xC3u, 0xC3u};
  _mockp.packet(pkt);

  auto i_pkt = dcc::make_idle_packet();

  TransmitP(3u);
  auto res = TransmitP(3u);

  dcc::tx::Timings::size_type offset = 0;

  ASSERT_TRUE(EvalPacket(res, pkt, offset))
    << "Malformed Packet data at " << offset;

  offset = 0;
  res = TransmitP(3u);

  ASSERT_TRUE(EvalPacket(res, i_pkt, offset))
    << "Malformed Packet data at " << offset;
}

TEST_F(TxTest, transmit_p_packet_after_packet) {
  dcc::Packet pkt{0xC3u, 0xC3u, 0xC3u};
  _mockp.packet(pkt);
  _mockp.packet(pkt);

  TransmitP(3u);
  auto res = TransmitP(3u);

  uint16_t offset = 0;

  ASSERT_TRUE(EvalPacket(res, pkt, offset))
    << "Malformed Packet data at " << offset;

  offset = 0;
  res = TransmitP(3u);

  ASSERT_TRUE(EvalPacket(res, pkt, offset))
    << "Malformed Packet data at " << offset;
}

TEST_F(TxTest, transmit_p_keep_idle) {
  auto i_pkt = dcc::make_idle_packet();

  auto res = TransmitP(3u);

  uint16_t offset = 0;

  ASSERT_TRUE(EvalPacket(res, i_pkt, offset))
    << "Malformed Idle Packet data at " << offset;

  offset = 0;

  ASSERT_TRUE(EvalPacket(res, i_pkt, offset))
    << "Malformed Idle Packet data at " << offset;
}