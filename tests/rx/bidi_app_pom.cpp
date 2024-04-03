#include "bidi_test.hpp"

TEST_F(BiDiTest, app_pom) {
  auto value{static_cast<uint8_t>(rand())};
  pom(value);
  _addrs.received = _addrs.primary;
  auto datagram{encode_datagram(make_datagram<Bits::_12>(0u, value))};
  EXPECT_CALL(*this, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  cutoutChannel2();
}

TEST_F(BiDiTest, app_pom_disabled_with_cv28_1) {
  _cvs[28uz - 1uz] = static_cast<uint8_t>(_cvs[28uz - 1uz] & 0b1111'11101u);
  _addrs.received = _addrs.primary;
  SetUp();

  pom(static_cast<uint8_t>(rand()));
  _addrs.received = _addrs.primary;
  EXPECT_CALL(*this, transmitBiDi(_)).Times(0);
  cutoutChannel2();
}