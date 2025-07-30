#include "tx_test.hpp"

TEST_F(TxTest, service_idle) {
  auto packet{dcc::make_reset_packet()};
  _packet_mock.init(_cfg, packet);
  _timings_mock.init(_cfg, packet);
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
