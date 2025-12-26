#include "rx_test.hpp"

using namespace dcc::bidi;

TEST_F(RxTest, app_pom) {
  auto cv_addr{RandomInterval<uint8_t>(0u, 255u)};
  auto value{RandomInterval<uint8_t>(0u, 255u)};

  EXPECT_CALL(_mock,
              readCv(Matcher<uint32_t>(cv_addr),
                     Matcher<uint8_t>(_),
                     Matcher<std::function<void(uint8_t)>>(_)))
    .WillOnce(InvokeArgument<2uz>(value));

  auto packet{make_cv_access_long_verify_packet(_addrs.primary, cv_addr)};
  Receive(packet)->LeaveCutout()->Execute()->Receive(packet);

  auto datagram{make_app_pom_datagram(value)};
  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  _mock.biDiChannel2();
}

TEST_F(RxTest, app_pom_disabled_with_cv28_1) {
  _cvs[28uz - 1uz] = static_cast<uint8_t>(_cvs[28uz - 1uz] & 0b1111'11101u);
  SetUp();

  auto cv_addr{RandomInterval<uint8_t>(0u, 255u)};
  auto value{RandomInterval<uint8_t>(0u, 255u)};

  EXPECT_CALL(_mock,
              readCv(Matcher<uint32_t>(cv_addr),
                     Matcher<uint8_t>(_),
                     Matcher<std::function<void(uint8_t)>>(_)))
    .WillOnce(InvokeArgument<2uz>(value));

  auto packet{make_cv_access_long_verify_packet(_addrs.primary, cv_addr)};
  Receive(packet)->LeaveCutout()->Execute()->Receive(packet);

  EXPECT_CALL(_mock, transmitBiDi(_)).Times(0);
  _mock.biDiChannel2();
}

TEST_F(RxTest, app_pom_reply_on_any_packets) {
  auto cv_addr{RandomInterval<uint8_t>(0u, 255u)};
  auto value{RandomInterval<uint8_t>(0u, 255u)};

  EXPECT_CALL(_mock,
              readCv(Matcher<uint32_t>(cv_addr),
                     Matcher<uint8_t>(_),
                     Matcher<std::function<void(uint8_t)>>(_)))
    .WillOnce(InvokeArgument<2uz>(value));

  auto packet{make_cv_access_long_verify_packet(_addrs.primary, cv_addr)};
  Receive(packet)->LeaveCutout()->Execute();

  auto other_packet_to_same_address{make_f0_f4_packet(_addrs.primary, 0b1u)};
  Receive(other_packet_to_same_address);

  auto datagram{make_app_pom_datagram(value)};
  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  _mock.biDiChannel2();
}

// https://github.com/ZIMO-Elektronik/DCC/issues/105
TEST_F(RxTest, app_pom_clear_internal_queue_on_unknown_cv_access_packet) {
  auto cv_addr{RandomInterval<uint8_t>(0u, 255u)};
  auto value{RandomInterval<uint8_t>(0u, 255u)};

  EXPECT_CALL(_mock,
              readCv(Matcher<uint32_t>(cv_addr),
                     Matcher<uint8_t>(_),
                     Matcher<std::function<void(uint8_t)>>(_)))
    .WillOnce(InvokeArgument<2uz>(value));

  auto packet{make_cv_access_long_verify_packet(_addrs.primary, cv_addr)};
  Receive(packet)->LeaveCutout()->Execute();

  // At this point in time there is an ID0 datagram in the internal queue.
  // Now send a new CV access command without invoking the callback. This
  // simulates a delay in a real application.
  EXPECT_CALL(_mock,
              readCv(Matcher<uint32_t>(cv_addr + 1u),
                     Matcher<uint8_t>(_),
                     Matcher<std::function<void(uint8_t)>>(_)));

  auto other_cv_packet{
    make_cv_access_long_verify_packet(_addrs.primary, cv_addr + 1u)};
  Receive(other_cv_packet)->LeaveCutout()->Execute()->Receive(other_cv_packet);

  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(acks))).Times(1);
  _mock.biDiChannel2();
}
