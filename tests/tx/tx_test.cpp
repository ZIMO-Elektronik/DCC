#include "tx_test.hpp"

TxTest::TxTest() {}

TxTest::~TxTest() {}

void TxTest::SetUp() {
  _packet_mock.init(_cfg);
  _timings_mock.init(_cfg);

  auto packet{dcc::make_idle_packet()};

  auto timings{dcc::tx::packet2timings(packet, _cfg)};

  auto bits{(_cfg.num_preamble +                  // Preamble
             std::size(packet) * (1uz + CHAR_BIT) // Start + data
             + 1uz) *                             // End
            2uz};

  // Idle packet
  for (auto i{0uz}; i < bits; ++i)
    EXPECT_ALL_EQ(timings[static_cast<dcc::tx::Timings::size_type>(i)],
                  _packet_mock.transmit(),
                  _timings_mock.transmit());

  // BiDi
  if (_cfg.flags.bidi) {
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
}
