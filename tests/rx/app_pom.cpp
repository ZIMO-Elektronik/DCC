#include "rx_test.hpp"

using namespace dcc::bidi;

TEST_F(RxTest, app_pom) {
  auto cv_addr{static_cast<uint8_t>(rand())};
  auto value{static_cast<uint8_t>(rand())};

  EXPECT_CALL(_mock, readCv(cv_addr, _)).WillRepeatedly(Return(value));

  Receive(dcc::make_cv_access_long_verify_packet(_addrs.primary, cv_addr));
  Execute();

  auto datagram{encode_datagram(make_datagram<Bits::_12>(0u, value))};
  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  _mock.cutoutChannel2();
}

TEST_F(RxTest, app_pom_disabled_with_cv28_1) {
  _cvs[28uz - 1uz] = static_cast<uint8_t>(_cvs[28uz - 1uz] & 0b1111'11101u);
  SetUp();

  auto cv_addr{static_cast<uint8_t>(rand())};
  auto value{static_cast<uint8_t>(rand())};

  EXPECT_CALL(_mock, readCv(cv_addr, _)).WillRepeatedly(Return(value));

  Receive(dcc::make_cv_access_long_verify_packet(_addrs.primary, cv_addr));
  Execute();

  EXPECT_CALL(_mock, transmitBiDi(_)).Times(0);
  _mock.cutoutChannel2();
}