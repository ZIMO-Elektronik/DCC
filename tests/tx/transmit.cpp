#include "tx_test.hpp"

TEST_F(TxTest, initial_idle_packet) {
  // See Setup()
}

TEST_F(TxTest, consecutive_idle_packets) {
  auto packet{dcc::make_idle_packet()};
  auto timings{dcc::tx::packet2timings(packet, _cfg)};
  auto bits{(_cfg.num_preamble +                  // Preamble
             std::size(packet) * (1uz + CHAR_BIT) // Start + data
             + 1uz) *                             // End
            2uz};
  for (auto i{0uz}; i < bits; ++i)
    EXPECT_ALL_EQ(timings[static_cast<dcc::tx::Timings::size_type>(i)],
                  _packet_mock.transmit(),
                  _timings_mock.transmit());
}

TEST_F(TxTest, consecutive_packets_without_cutout) {
  _cfg.flags.bidi = false;
  SetUp();

  auto packet{dcc::make_idle_packet()};
  auto timings{dcc::tx::packet2timings(packet, _cfg)};
  auto bits{(_cfg.num_preamble +                  // Preamble
             std::size(packet) * (1uz + CHAR_BIT) // Start + data
             + 1uz) *                             // End
            2uz};
  for (auto i{0uz}; i < bits; ++i)
    EXPECT_ALL_EQ(timings[static_cast<dcc::tx::Timings::size_type>(i)],
                  _packet_mock.transmit(),
                  _timings_mock.transmit());
  for (auto i{0uz}; i < bits; ++i)
    EXPECT_ALL_EQ(timings[static_cast<dcc::tx::Timings::size_type>(i)],
                  _packet_mock.transmit(),
                  _timings_mock.transmit());
}

TEST_F(TxTest, consecutive_packets_with_cutout) {
  {
    auto packet{dcc::make_function_group_f12_f9_packet(3u, 0b0000'1100u)};
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
  }

  {
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
  }

  {
    auto packet{dcc::make_idle_packet()};
    auto timings{dcc::tx::packet2timings(packet, _cfg)};
    auto bits{(_cfg.num_preamble +                  // Preamble
               std::size(packet) * (1uz + CHAR_BIT) // Start + data
               + 1uz) *                             // End
              2uz};
    for (auto i{0uz}; i < bits; ++i)
      EXPECT_ALL_EQ(timings[static_cast<dcc::tx::Timings::size_type>(i)],
                    _packet_mock.transmit(),
                    _timings_mock.transmit());
  }
}
