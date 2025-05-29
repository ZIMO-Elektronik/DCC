#include "tx_test.hpp"

TEST_F(TxTest, transmit_packet) {
  auto packet{dcc::make_idle_packet()};
  EXPECT_TRUE(_packet_mock.packet(packet));

  auto timings{dcc::tx::packet2timings(packet, _cfg)};

  auto bits{(_cfg.num_preamble +                  // Preamble
             std::size(packet) * (1uz + CHAR_BIT) // Start + data
             + 1uz) *                             // End
            2uz};

  // Idle packet
  for (auto i{0uz}; i < bits; ++i)
    EXPECT_EQ(timings[static_cast<dcc::tx::Timings::size_type>(i)],
              _packet_mock.transmit());

  // BiDi
  EXPECT_EQ(static_cast<dcc::tx::Timings::value_type>(dcc::bidi::Timing::TCS),
            _packet_mock.transmit());
  EXPECT_EQ(static_cast<dcc::tx::Timings::value_type>(dcc::bidi::Timing::TTS1 -
                                                      dcc::bidi::Timing::TCS),
            _packet_mock.transmit());
  EXPECT_EQ(static_cast<dcc::tx::Timings::value_type>(dcc::bidi::Timing::TTS2 -
                                                      dcc::bidi::Timing::TTS1),
            _packet_mock.transmit());
  EXPECT_EQ(static_cast<dcc::tx::Timings::value_type>(dcc::bidi::Timing::TTC2 -
                                                      dcc::bidi::Timing::TTS2),
            _packet_mock.transmit());
  EXPECT_EQ(static_cast<dcc::tx::Timings::value_type>(dcc::bidi::Timing::TCE -
                                                      dcc::bidi::Timing::TTC2),
            _packet_mock.transmit());

  // Idle packet
  for (auto i{0uz}; i < bits; ++i)
    EXPECT_EQ(timings[static_cast<dcc::tx::Timings::size_type>(i)],
              _packet_mock.transmit());
}

TEST_F(TxTest, transmit_timings) {
  auto packet{dcc::make_idle_packet()};
  EXPECT_TRUE(_timings_mock.packet(packet));

  auto timings{dcc::tx::packet2timings(packet, _cfg)};

  auto bits{(_cfg.num_preamble +                  // Preamble
             std::size(packet) * (1uz + CHAR_BIT) // Start + data
             + 1uz) *                             // End
            2uz};

  // Idle packet
  for (auto i{0uz}; i < bits; ++i)
    EXPECT_EQ(timings[static_cast<dcc::tx::Timings::size_type>(i)],
              _timings_mock.transmit());

  // BiDi
  EXPECT_EQ(static_cast<dcc::tx::Timings::value_type>(dcc::bidi::Timing::TCS),
            _timings_mock.transmit());
  EXPECT_EQ(static_cast<dcc::tx::Timings::value_type>(dcc::bidi::Timing::TTS1 -
                                                      dcc::bidi::Timing::TCS),
            _timings_mock.transmit());
  EXPECT_EQ(static_cast<dcc::tx::Timings::value_type>(dcc::bidi::Timing::TTS2 -
                                                      dcc::bidi::Timing::TTS1),
            _timings_mock.transmit());
  EXPECT_EQ(static_cast<dcc::tx::Timings::value_type>(dcc::bidi::Timing::TTC2 -
                                                      dcc::bidi::Timing::TTS2),
            _timings_mock.transmit());
  EXPECT_EQ(static_cast<dcc::tx::Timings::value_type>(dcc::bidi::Timing::TCE -
                                                      dcc::bidi::Timing::TTC2),
            _timings_mock.transmit());

  // Idle packet
  for (auto i{0uz}; i < bits; ++i)
    EXPECT_EQ(timings[static_cast<dcc::tx::Timings::size_type>(i)],
              _timings_mock.transmit());
}
