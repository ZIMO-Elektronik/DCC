#include <array>
#include "rx_test.hpp"

using namespace dcc::bidi;
using namespace std::chrono_literals;

TEST_F(RxTest, app_search_basic_address) {
  // Does not require CV28:1
  _cvs[28uz - 1uz] = static_cast<uint8_t>(_cvs[28uz - 1uz] & 0b1111'11101u);
  SetUp();

  // Make sure to get past backoff (see RCN-218)
  for (auto i{0.0}; i < 30.0 / 10E-3; ++i)
    LeaveCutout()->Execute()->Receive(
      dcc::make_binary_state_short_packet(0u, 2u, false));

  // Make datagram
  auto datagram{make_app_search_datagram(_addrs.primary, 0u, 0u)};

  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  _mock.biDiChannel2();
}

TEST_F(RxTest, app_search_extended_address) {
  _addrs.primary = {.value = 3000u, .type = dcc::Address::ExtendedLoco};
  dcc::encode_address(_addrs.primary, begin(_cvs) + 17 - 1);
  _cvs[29uz - 1uz] = _cvs[29uz - 1uz] | ztl::mask<5u>;
  SetUp();

  // Make sure to get past backoff (see RCN-218)
  for (auto i{0.0}; i < 30.0 / 10E-3; ++i)
    LeaveCutout()->Execute()->Receive(
      dcc::make_binary_state_short_packet(0u, 2u, false));

  // Make datagram
  auto datagram{make_app_search_datagram(_addrs.primary, 0u, 0u)};

  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  _mock.biDiChannel2();
}

TEST_F(RxTest, app_search_consist_address) {
  _cvs[19uz - 1uz] = static_cast<uint8_t>(_addrs.consist);
  SetUp();

  // Make sure to get past backoff (see RCN-218)
  for (auto i{0.0}; i < 30.0 / 10E-3; ++i)
    LeaveCutout()->Execute()->Receive(
      dcc::make_binary_state_short_packet(0u, 2u, false));

  // Make datagram
  auto datagram{make_app_search_datagram(_addrs.consist, _cvs[19uz - 1uz], 0u)};

  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  _mock.biDiChannel2();
}

TEST_F(
  RxTest,
  app_search_time_is_from_first_packet_regardless_of_which_packet_is_answered) {
  SetUp();

  // Make sure to get past backoff (see RCN-218)
  for (auto i{0.0}; i < 30.0 / 10E-3; ++i)
    LeaveCutout()->Execute()->Receive(
      dcc::make_binary_state_short_packet(0u, 2u, false));

  // Make datagram
  auto datagram{make_app_search_datagram(_addrs.primary, 0u, 0u)};

  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  _mock.biDiChannel2();

  std::this_thread::sleep_for(1s);

  // Make sure to get past backoff (see RCN-218)
  for (auto i{0.0}; i < 30.0 / 10E-3; ++i)
    LeaveCutout()->Execute()->Receive(
      dcc::make_binary_state_short_packet(0u, 2u, false));

  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  _mock.biDiChannel2();
}
