#include "tx_test.hpp"

TEST_F(TxTest, address) {
  _cfg.flags.bidi = false;
  SetUp();

  dcc::Address addr{.value = 3u, .type = dcc::Address::BasicLoco};

  auto packet{dcc::make_128_speed_step_control_packet(addr, 0x00u)};
  EXPECT_ALL_TRUE(_packet_mock.packet(packet), _timings_mock.packet(packet));
  auto timings{dcc::tx::packet2timings(packet, _cfg)};
  auto bits{(_cfg.num_preamble +                  // Preamble
             std::size(packet) * (1uz + CHAR_BIT) // Start + data
             + 1uz) *                             // End
            2uz};

  for (auto i{0uz}; i < bits; ++i)
    EXPECT_ALL_EQ(timings[static_cast<dcc::tx::Timings::size_type>(i)],
                  _packet_mock.transmit(),
                  _timings_mock.transmit());

  // Address is updated only when the next packet starts (after the cutout)
  _packet_mock.transmit();
  _timings_mock.transmit();

  EXPECT_ALL_EQ(addr, _packet_mock.address(), _timings_mock.address());
}

TEST_F(TxTest, idle_address) {
  _cfg.flags.bidi = false;
  SetUp();

  auto packet{dcc::make_idle_packet()};
  auto timings{dcc::tx::packet2timings(packet, _cfg)};
  auto bits{(_cfg.num_preamble +                  // Preamble
             std::size(packet) * (1uz + CHAR_BIT) // Start + data
             + 1uz) *                             // End
            2uz};

  // Try 3 idle packets
  for (auto i{0uz}; i < bits * 3uz; ++i) {
    EXPECT_ALL_EQ(timings[static_cast<dcc::tx::Timings::size_type>(i % bits)],
                  _packet_mock.transmit(),
                  _timings_mock.transmit());
    EXPECT_ALL_EQ(dcc::decode_address(packet),
                  _packet_mock.address(),
                  _timings_mock.address());
  }

  // Address is updated only when the next packet starts (after the cutout)
  _packet_mock.transmit();
  _timings_mock.transmit();

  EXPECT_ALL_EQ(dcc::decode_address(packet),
                _packet_mock.address(),
                _timings_mock.address());
}

TEST_F(TxTest, address_bidi_cutout) {
  dcc::Address addr{.value = 3u, .type = dcc::Address::BasicLoco};

  auto packet{dcc::make_128_speed_step_control_packet(addr, 0x00u)};
  EXPECT_ALL_TRUE(_packet_mock.packet(packet), _timings_mock.packet(packet));
  auto timings{dcc::tx::packet2timings(packet, _cfg)};
  auto bits{(_cfg.num_preamble +                  // Preamble
             std::size(packet) * (1uz + CHAR_BIT) // Start + data
             + 1uz) *                             // End
            2uz};

  // Consume all timings, including BiDi
  for (auto i{0uz}; i < bits; ++i)
    EXPECT_ALL_EQ(timings[static_cast<dcc::tx::Timings::size_type>(i)],
                  _packet_mock.transmit(),
                  _timings_mock.transmit());
  EXPECT_ALL_EQ(
    static_cast<dcc::tx::Timings::value_type>(dcc::bidi::Timing::TCS),
    _packet_mock.transmit(),
    _timings_mock.transmit());
  EXPECT_ALL_EQ(static_cast<dcc::tx::Timings::value_type>(
                  dcc::bidi::Timing::TTS1 - dcc::bidi::Timing::TCS),
                _packet_mock.transmit(),
                _timings_mock.transmit());
  EXPECT_ALL_EQ(static_cast<dcc::tx::Timings::value_type>(
                  dcc::bidi::Timing::TTS2 - dcc::bidi::Timing::TTS1),
                _packet_mock.transmit(),
                _timings_mock.transmit());
  EXPECT_ALL_EQ(static_cast<dcc::tx::Timings::value_type>(
                  dcc::bidi::Timing::TTC2 - dcc::bidi::Timing::TTS2),
                _packet_mock.transmit(),
                _timings_mock.transmit());
  EXPECT_ALL_EQ(static_cast<dcc::tx::Timings::value_type>(
                  dcc::bidi::Timing::TCE - dcc::bidi::Timing::TTC2),
                _packet_mock.transmit(),
                _timings_mock.transmit());

  // Address is updated only when the next packet starts (after the cutout)
  _packet_mock.transmit();
  _timings_mock.transmit();

  EXPECT_ALL_EQ(addr, _packet_mock.address(), _timings_mock.address());
}
