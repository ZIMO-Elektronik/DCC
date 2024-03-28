#include "bidi_test.hpp"

TEST_F(BiDiTest, app_adr_alternate_id1_id2) {
  _addrs.received = _addrs.primary;

  // Encode address
  std::array<uint8_t, 2uz> adr;
  encode_address(_addrs.primary, begin(adr));

  // Make datagram
  auto adr_high{
    encode_datagram(make_datagram<Bits::_12>(1u, adr[0uz] & 0b1011'1111u))};
  auto adr_low{encode_datagram(make_datagram<Bits::_12>(2u, adr[1uz]))};

  InSequence s;
  for (auto i{0uz}; i < 10uz; ++i) {
    EXPECT_CALL(*this, transmitBiDi(DatagramMatcher(adr_high))).Times(1);
    cutoutChannel1();

    EXPECT_CALL(*this, transmitBiDi(DatagramMatcher(adr_low))).Times(1);
    cutoutChannel1();
  }
}

TEST_F(BiDiTest, app_adr_disabled_with_cv28_0) {
  _cvs[28uz - 1uz] = static_cast<uint8_t>(_cvs[28uz - 1uz] & 0b1111'11110u);
  _addrs.received = _addrs.primary;
  SetUp();

  EXPECT_CALL(*this, transmitBiDi(_)).Times(0);
  cutoutChannel1();
}