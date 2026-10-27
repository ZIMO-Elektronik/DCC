#include "../utility.hpp"
#include "tx_test.hpp"

TEST_F(TxTest, track_outputs_first_halfbit_P_gt_N) {
  EXPECT_CALL(_packet_mock, trackOutputs(true, false)).Times(1);
  EXPECT_CALL(_timings_mock, trackOutputs(true, false)).Times(1);
  _packet_mock.transmit();
  _timings_mock.transmit();
}

TEST_F(TxTest,
       track_outputs_polarity_does_not_change_across_packets_with_bidi) {
  auto packet{dcc::make_f0_f4_packet(random_interval<uint8_t>(),
                                     random_interval<uint8_t>())};
  auto timings{dcc::tx::packet2timings(packet, _cfg)};
  auto bits{(_cfg.num_preamble +                  // Preamble
             std::size(packet) * (1uz + CHAR_BIT) // Start + data
             + 1uz) *                             // End
            2uz};
  _packet_mock.packet(packet);
  _timings_mock.packet(packet);

  for (auto i{0uz}; i < bits; ++i)
    EXPECT_ALL_EQ(timings[static_cast<dcc::tx::Timings::size_type>(i)],
                  _packet_mock.transmit(),
                  _timings_mock.transmit());

  // BiDi
  for (auto i{0uz}; i < 5uz; ++i) {
    _packet_mock.transmit();
    _timings_mock.transmit();
  }

  EXPECT_CALL(_packet_mock, trackOutputs(true, false)).Times(1);
  EXPECT_CALL(_timings_mock, trackOutputs(true, false)).Times(1);
  _packet_mock.transmit();
  _timings_mock.transmit();
}

TEST_F(TxTest,
       track_outputs_polarity_does_not_change_across_packets_without_bidi) {
  _cfg.flags.bidi = false;
  SetUp();

  auto packet{dcc::make_f0_f4_packet(random_interval<uint8_t>(),
                                     random_interval<uint8_t>())};
  auto timings{dcc::tx::packet2timings(packet, _cfg)};
  auto bits{(_cfg.num_preamble +                  // Preamble
             std::size(packet) * (1uz + CHAR_BIT) // Start + data
             + 1uz) *                             // End
            2uz};
  _packet_mock.packet(packet);
  _timings_mock.packet(packet);

  for (auto i{0uz}; i < bits; ++i)
    EXPECT_ALL_EQ(timings[static_cast<dcc::tx::Timings::size_type>(i)],
                  _packet_mock.transmit(),
                  _timings_mock.transmit());

  EXPECT_CALL(_packet_mock, trackOutputs(true, false)).Times(1);
  EXPECT_CALL(_timings_mock, trackOutputs(true, false)).Times(1);
  _packet_mock.transmit();
  _timings_mock.transmit();
}
