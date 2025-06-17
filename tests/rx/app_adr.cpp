#include "rx_test.hpp"

using namespace dcc::bidi;

TEST_F(RxTest, app_adr_alternate_primary_id1_id2) {
  // Make datagram
  auto adr_high{encode_datagram(make_datagram<Bits::_12>(1u, 0u))};
  auto adr_low{encode_datagram(
    make_datagram<Bits::_12>(2u, static_cast<uint8_t>(_addrs.primary)))};

  InSequence s;
  for (auto i{0uz}; i < 10uz; ++i) {
    // Send whatever packet to get last received address to match primary
    Receive(dcc::make_function_group_f4_f0_packet(_addrs.primary, 10u));

    EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(adr_high))).Times(1);
    Execute();
    _mock.biDiChannel1();

    EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(adr_low))).Times(1);
    Execute();
    _mock.biDiChannel1();
  }
}

TEST_F(RxTest, app_adr_alternate_logon_id1_id2) {
  Logon();

  // Make datagram
  auto adr_high{encode_datagram(
    make_datagram<Bits::_12>(1u, 0x80u | (_addrs.logon & 0x3F00u) >> 8u))};
  auto adr_low{
    encode_datagram(make_datagram<Bits::_12>(2u, _addrs.logon & 0x00FFu))};

  InSequence s;
  for (auto i{0uz}; i < 10uz; ++i) {
    // Send whatever packet to get last received address to match logon
    Receive(dcc::make_function_group_f4_f0_packet(_addrs.logon, 10u));

    EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(adr_high))).Times(1);
    Execute();
    _mock.biDiChannel1();

    EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(adr_low))).Times(1);
    Execute();
    _mock.biDiChannel1();
  }
}

TEST_F(RxTest, app_adr_disabled_with_cv28_0) {
  _cvs[28uz - 1uz] = static_cast<uint8_t>(_cvs[28uz - 1uz] & 0b1111'11110u);
  SetUp();

  // Send whatever packet to get last received address to match primary
  Receive(dcc::make_function_group_f4_f0_packet(_addrs.primary, 10u));

  EXPECT_CALL(_mock, transmitBiDi(_)).Times(0);
  Execute();
  _mock.biDiChannel1();
}

TEST_F(RxTest, app_adr_consist) {
  _cvs[19uz - 1uz] = static_cast<uint8_t>(_addrs.consist);
  SetUp();

  // Make datagram
  auto adr_high{encode_datagram(make_datagram<Bits::_12>(1u, 0b0110'0000u))};
  auto adr_low{encode_datagram(
    make_datagram<Bits::_12>(2u, static_cast<uint8_t>(_addrs.consist)))};

  // Send whatever packet to get last received address to match consist
  Receive(dcc::make_function_group_f4_f0_packet(_addrs.consist, 10u));

  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(adr_high))).Times(1);
  Execute();
  _mock.biDiChannel1();

  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(adr_low))).Times(1);
  Execute();
  _mock.biDiChannel1();
}

TEST_F(RxTest, app_adr_long_consist) {
  _cvs[19uz - 1uz] = 83u;
  _cvs[20uz - 1uz] = 12u;
  _addrs.consist = static_cast<dcc::Address::value_type>(
    100u * (_cvs[20uz - 1uz] & 0b0111'1111u) +
    (_cvs[19uz - 1uz] & 0b0111'1111u));
  SetUp();

  // Make datagram
  auto adr_high{encode_datagram(
    make_datagram<Bits::_12>(1u, 0x80u | (_addrs.consist & 0x3F00u) >> 8u))};
  auto adr_low{
    encode_datagram(make_datagram<Bits::_12>(2u, _addrs.consist & 0x00FFu))};

  // Send whatever packet to get last received address to match consist
  Receive(dcc::make_function_group_f4_f0_packet(_addrs.consist, 10u));

  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(adr_high))).Times(1);
  Execute();
  _mock.biDiChannel1();

  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(adr_low))).Times(1);
  Execute();
  _mock.biDiChannel1();
}

TEST_F(RxTest, app_adr_broadcast) {
  // Make datagram
  auto adr_high{encode_datagram(make_datagram<Bits::_12>(1u, 0u))};
  auto adr_low{encode_datagram(
    make_datagram<Bits::_12>(2u, static_cast<uint8_t>(_addrs.primary)))};

  InSequence s;
  for (auto i{0uz}; i < 10uz; ++i) {
    // Broadcast
    Receive(dcc::make_speed_and_direction_packet(0u, 0u));

    EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(adr_high))).Times(0);
    Execute();
    _mock.biDiChannel1();

    EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(adr_low))).Times(0);
    Execute();
    _mock.biDiChannel1();
  }
}
