#include "rx_test.hpp"

using namespace dcc::bidi;

TEST_F(RxTest, app_adr_alternate_primary_address) {
  // Make datagram
  auto adr_high{make_app_adr_high_datagram(_addrs.primary)};
  auto adr_low{make_app_adr_low_datagram(_addrs.primary)};

  // Send whatever packet to get last received address to match primary
  auto packet{make_f0_f4_packet(_addrs.primary, 10u)};
  Receive(packet);

  InSequence s;
  for (auto i{0uz}; i < 10uz; ++i) {
    LeaveCutout()->Execute()->Receive(packet);

    EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(adr_high))).Times(1);
    _mock.biDiChannel1();

    EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(adr_low))).Times(1);
    _mock.biDiChannel1();
  }
}

TEST_F(RxTest, app_adr_alternate_logon_address) {
  Logon();

  // Make datagram
  auto adr_high{make_app_adr_high_datagram(_addrs.logon)};
  auto adr_low{make_app_adr_low_datagram(_addrs.logon)};

  // Send whatever packet to get last received address to match primary
  auto packet{make_f0_f4_packet(_addrs.primary, 10u)};
  Receive(packet);

  InSequence s;
  for (auto i{0uz}; i < 10uz; ++i) {
    LeaveCutout()->Execute()->Receive(packet);

    EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(adr_high))).Times(1);
    _mock.biDiChannel1();

    EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(adr_low))).Times(1);
    _mock.biDiChannel1();
  }
}

TEST_F(RxTest, app_adr_disabled_with_cv28_0) {
  _cvs[28uz - 1uz] = static_cast<uint8_t>(_cvs[28uz - 1uz] & 0b1111'11110u);
  SetUp();

  // Send whatever packet to get last received address to match primary
  Receive(make_f0_f4_packet(_addrs.primary, 10u));

  EXPECT_CALL(_mock, transmitBiDi(_)).Times(0);
  Execute();
  _mock.biDiChannel1();
}

TEST_F(RxTest, app_adr_alternate_consist_address) {
  _cvs[19uz - 1uz] = static_cast<uint8_t>(_addrs.consist);
  SetUp();

  // Make datagram
  auto adr_high{make_app_adr_high_datagram(_addrs.consist, _cvs[19uz - 1uz])};
  auto adr_low{make_app_adr_low_datagram(_addrs.consist, _cvs[19uz - 1uz])};

  // Send whatever packet to get last received address to match primary
  auto packet{make_f0_f4_packet(_addrs.consist, 10u)};
  Receive(packet);

  InSequence s;
  for (auto i{0uz}; i < 10uz; ++i) {
    LeaveCutout()->Execute()->Receive(packet);

    EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(adr_high))).Times(1);
    _mock.biDiChannel1();

    EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(adr_low))).Times(1);
    _mock.biDiChannel1();
  }
}

TEST_F(RxTest, app_adr_alternate_long_consist_address) {
  _cvs[19uz - 1uz] = 83u;
  _cvs[20uz - 1uz] = 12u;
  _addrs.consist = static_cast<dcc::Address::value_type>(
    100u * (_cvs[20uz - 1uz] & 0b0111'1111u) +
    (_cvs[19uz - 1uz] & 0b0111'1111u));
  SetUp();

  // Make datagram
  auto adr_high{make_app_adr_high_datagram(_addrs.consist, _cvs[19uz - 1uz])};
  auto adr_low{make_app_adr_low_datagram(_addrs.consist, _cvs[19uz - 1uz])};

  // Send whatever packet to get last received address to match primary
  auto packet{make_f0_f4_packet(_addrs.consist, 10u)};
  Receive(packet);

  InSequence s;
  for (auto i{0uz}; i < 10uz; ++i) {
    LeaveCutout()->Execute()->Receive(packet);

    EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(adr_high))).Times(1);
    _mock.biDiChannel1();

    EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(adr_low))).Times(1);
    _mock.biDiChannel1();
  }
}

TEST_F(RxTest, app_adr_broadcast) {
  // Make datagram
  auto adr_high{make_app_adr_high_datagram(_addrs.primary)};
  auto adr_low{make_app_adr_low_datagram(_addrs.primary)};

  // Broadcast
  auto packet{dcc::make_speed_and_direction_packet(0u, 0u)};
  Receive(packet);

  InSequence s;
  for (auto i{0uz}; i < 10uz; ++i) {
    LeaveCutout()->Execute()->Receive(packet);

    EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(adr_high))).Times(0);
    _mock.biDiChannel1();

    EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(adr_low))).Times(0);
    _mock.biDiChannel1();
  }
}
