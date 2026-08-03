
#include "rx_test.hpp"

using namespace dcc::bidi;

TEST_F(RxTest, app_xpom) {
  auto ss{RandomInterval<uint8_t>(0u, 3u)};
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  std::array<uint8_t, 4uz> cv_values{0u, 1u, 2u, 3u};
  auto packet{make_cv_access_xpom_verify_packet(_addrs.primary, ss, cv_addr)};

  InSequence s;
  EXPECT_CALL(_mock, readCv(cv_addr + 0u)).WillOnce(Return(cv_values[0uz]));
  EXPECT_CALL(_mock, readCv(cv_addr + 1u)).WillOnce(Return(cv_values[1uz]));
  EXPECT_CALL(_mock, readCv(cv_addr + 2u)).WillOnce(Return(cv_values[2uz]));
  EXPECT_CALL(_mock, readCv(cv_addr + 3u)).WillOnce(Return(cv_values[3uz]));
  Receive(packet)->LeaveCutout()->Execute()->Receive(packet);

  auto datagram{make_app_xpom_datagram(ss, cv_values)};
  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  _mock.biDiChannel2();
}

// https://github.com/ZIMO-Elektronik/DCC/issues/170
TEST_F(RxTest, app_xpom_has_higher_priority_than_app_pom) {
  auto ss{RandomInterval<uint8_t>(0u, 3u)};
  auto cv_addr{RandomInterval(0u, smath::pow(2u, 10u) - 1u)};
  std::array<uint8_t, 4uz> cv_values{0u, 1u, 2u, 3u};
  auto pom_packet{make_cv_access_long_verify_packet(_addrs.primary, cv_addr)};
  auto xpom_packet{
    make_cv_access_xpom_verify_packet(_addrs.primary, ss, cv_addr)};

  // Leave a ID0 app:pom datagram in the queue
  EXPECT_CALL(_mock,
              readCv(Matcher<uint32_t>(cv_addr),
                     Matcher<uint8_t>(_),
                     Matcher<std::function<void(uint8_t)>>(_)))
    .WillOnce(InvokeArgument<2uz>(cv_values[0uz]));
  ReceiveAndExecute(pom_packet);

  InSequence s;
  EXPECT_CALL(_mock, readCv(cv_addr + 0u)).WillOnce(Return(cv_values[0uz]));
  EXPECT_CALL(_mock, readCv(cv_addr + 1u)).WillOnce(Return(cv_values[1uz]));
  EXPECT_CALL(_mock, readCv(cv_addr + 2u)).WillOnce(Return(cv_values[2uz]));
  EXPECT_CALL(_mock, readCv(cv_addr + 3u)).WillOnce(Return(cv_values[3uz]));
  Receive(xpom_packet)->LeaveCutout()->Execute()->Receive(xpom_packet);

  auto datagram{make_app_xpom_datagram(ss, cv_values)};
  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  _mock.biDiChannel2();
}
