#include "rx_test.hpp"

using namespace dcc::bidi;

TEST_F(RxTest, app_dyn) {
  // Send whatever packet to get last received address to match primary
  ReceiveAndExecute(make_f0_f4_packet(_addrs.primary, 10u))->LeaveCutout();

  // Add more datagrams than would fit the queue
  for (auto i{0uz}; i < DCC_RX_BIDI_APP_DYN_DEQUE_SIZE + 1uz; ++i)
    _mock.appDyn(DirectionStatusByte{static_cast<uint8_t>(i)});

  EnterCutout();

  auto first{make_app_dyn_datagram(0u, 27u)};
  auto second{make_app_dyn_datagram(1u, 27u)};
  std::vector<uint8_t> datagram;
  std::ranges::copy(first, std::back_inserter(datagram));
  std::ranges::copy(second, std::back_inserter(datagram));
  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  _mock.biDiChannel2();
}

TEST_F(RxTest, dont_transmit_following_foreign_address) {
  // Send whatever packet to foreign address
  Receive(dcc::make_f0_f4_packet(1817u, 10u));

  // Add more datagrams than would fit the queue
  for (auto i{0uz}; i < DCC_RX_BIDI_APP_DYN_DEQUE_SIZE + 1uz; ++i)
    _mock.appDyn(DirectionStatusByte{static_cast<uint8_t>(i)});

  EXPECT_CALL(_mock, transmitBiDi(_)).Times(0);
  _mock.biDiChannel2();
}
