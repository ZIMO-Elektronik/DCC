#include "rx_test.hpp"

using namespace dcc::bidi;

TEST_F(RxTest, app_dyn) {
  // Send whatever packet to get last received address to match primary
  Receive(make_f0_f4_packet(_addrs.primary, 10u));

  EXPECT_LT(DCC_RX_BIDI_DEQUE_SIZE, 42uz);

  // Add more datagrams than would fit the queue
  for (auto i{0uz}; i < 42u; ++i)
    _mock.datagram(DirectionStatusByte{static_cast<uint8_t>(i)});

  uint8_t i{0u};

  {
    // First datagram after deque release is always QoS
    auto first{make_app_dyn_datagram(0u, 7u)};
    auto second{make_app_dyn_datagram(i++, 27u)};
    std::vector<uint8_t> datagram;
    std::ranges::copy(first, std::back_inserter(datagram));
    std::ranges::copy(second, std::back_inserter(datagram));
    EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
    _mock.biDiChannel2();
  }

  {
    auto third{make_app_dyn_datagram(i++, 27u)};
    auto fourth{make_app_dyn_datagram(i++, 27u)};
    std::vector<uint8_t> datagram;
    std::ranges::copy(third, std::back_inserter(datagram));
    std::ranges::copy(fourth, std::back_inserter(datagram));
    EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
    _mock.biDiChannel2();
  }

  {
    auto fifth{make_app_dyn_datagram(i++, 27u)};
    auto sixt{make_app_dyn_datagram(i++, 27u)};
    std::vector<uint8_t> datagram;
    std::ranges::copy(fifth, std::back_inserter(datagram));
    std::ranges::copy(sixt, std::back_inserter(datagram));
    EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
    _mock.biDiChannel2();
  }

  {
    // Last datagram which fit into deque
    auto seventh{make_app_dyn_datagram(i++, 27u)};
    EXPECT_EQ(DCC_RX_BIDI_DEQUE_SIZE, 7uz);
    std::vector<uint8_t> datagram;
    std::ranges::copy(seventh, std::back_inserter(datagram));
    EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
    _mock.biDiChannel2();
  }

  {
    // Deque is empty again at this point
    EXPECT_CALL(_mock, transmitBiDi(_)).Times(0);
    _mock.biDiChannel2();
  }
}

TEST_F(RxTest, dont_transmit_following_foreign_address) {
  // Send whatever packet to foreign address
  Receive(dcc::make_f0_f4_packet(1817u, 10u));

  // Add more datagrams than would fit the queue
  for (auto i{0uz}; i < 42u; ++i)
    _mock.datagram(DirectionStatusByte{static_cast<uint8_t>(i)});

  EXPECT_CALL(_mock, transmitBiDi(_)).Times(0);
  _mock.biDiChannel2();
}
