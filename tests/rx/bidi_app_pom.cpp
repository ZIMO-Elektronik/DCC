#include "bidi_test.hpp"

TEST_F(BiDiTest, app_pom) {
  uint8_t value{42u};
  pom(value);
  addrs_.received = addrs_.primary;
  auto datagram{encode_datagram(make_datagram<Bits::_12>(0u, value))};
  EXPECT_CALL(*this, transmitBiDi(DatagramMatcher(datagram))).Times(Exactly(1));
  cutoutChannel2();
}