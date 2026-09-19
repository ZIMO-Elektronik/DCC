#include "rx_test.hpp"

using namespace dcc::bidi;

TEST_F(RxTest, app_pom) {
  auto cv_addr{RandomInterval<uint8_t>(0u, 255u)};
  auto byte{RandomInterval<uint8_t>(0u, 255u)};

  auto packet{make_cv_access_long_verify_packet(_addrs.primary, cv_addr)};
  EXPECT_CALL(_mock, readCv(cv_addr, _, A<std::function<void(uint8_t)>>()))
    .WillOnce(InvokeArgument<2uz>(byte));
  Receive(packet)->LeaveCutout()->Execute()->Receive(packet);

  auto datagram{make_app_pom_datagram(byte)};
  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  _mock.biDiChannel2();
}

TEST_F(RxTest, app_pom_reply_to_cv_access_long_write_bit_operations_mode) {
  // Don't write any CV which might trigger config (e.g. 1, 28, ...)!
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto byte{RandomInterval<uint8_t>(0u, 255u)};
  auto pos{RandomInterval(0u, 7u)};
  auto bit{static_cast<bool>(byte & (1u << pos))};
  auto packet{
    make_cv_access_long_write_packet(_addrs.primary, cv_addr, bit, pos)};

  // First writes the bit
  EXPECT_CALL(_mock, writeCv(cv_addr, bit, pos, A<std::function<void(bool)>>()))
    .WillOnce(InvokeArgument<3uz>(bit));
  // ... then reads the entire byte
  EXPECT_CALL(_mock, readCv(cv_addr, 0u, A<std::function<void(uint8_t)>>()))
    .WillOnce(InvokeArgument<2uz>(byte));
  ReceiveAndExecuteTwice(packet);

  // ... and generates an ID0 datagram
  auto datagram{make_app_pom_datagram(byte)};
  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  Receive(packet);
  _mock.biDiChannel2();
}

TEST_F(RxTest, app_pom_disabled_with_cv28_1) {
  _cvs[28uz - 1uz] = static_cast<uint8_t>(_cvs[28uz - 1uz] & 0b1111'11101u);
  SetUp();

  auto cv_addr{RandomInterval<uint8_t>(0u, 255u)};
  auto byte{RandomInterval<uint8_t>(0u, 255u)};
  auto packet{make_cv_access_long_verify_packet(_addrs.primary, cv_addr)};
  EXPECT_CALL(_mock, readCv(cv_addr, _, A<std::function<void(uint8_t)>>()))
    .WillOnce(InvokeArgument<2uz>(byte));
  Receive(packet)->LeaveCutout()->Execute()->Receive(packet);

  EXPECT_CALL(_mock, transmitBiDi(_)).Times(0);
  _mock.biDiChannel2();
}

TEST_F(RxTest, app_pom_replies_to_all_loco_packets) {
  auto cv_addr{RandomInterval<uint8_t>(0u, 255u)};
  auto byte{RandomInterval<uint8_t>(0u, 255u)};
  auto packet{make_cv_access_long_verify_packet(_addrs.primary, cv_addr)};
  EXPECT_CALL(_mock, readCv(cv_addr, _, A<std::function<void(uint8_t)>>()))
    .WillOnce(InvokeArgument<2uz>(byte));
  Receive(packet)->LeaveCutout()->Execute();

  auto other_packet_to_same_address{make_f0_f4_packet(_addrs.primary, 0b1u)};
  Receive(other_packet_to_same_address);

  auto datagram{make_app_pom_datagram(byte)};
  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  _mock.biDiChannel2();
}

// https://github.com/ZIMO-Elektronik/DCC/issues/105
TEST_F(RxTest, app_pom_clear_internal_queue_on_unknown_cv_access_packet) {
  auto cv_addr{RandomInterval<uint8_t>(0u, 255u)};
  auto byte{RandomInterval<uint8_t>(0u, 255u)};
  auto packet{make_cv_access_long_verify_packet(_addrs.primary, cv_addr)};
  EXPECT_CALL(_mock, readCv(cv_addr, _, A<std::function<void(uint8_t)>>()))
    .WillOnce(InvokeArgument<2uz>(byte));
  Receive(packet)->LeaveCutout()->Execute();

  // At this point in time there is an ID0 datagram in the internal queue.
  // Now send a new CV access command without invoking the callback. This
  // simulates a delay in a real application.
  EXPECT_CALL(_mock,
              readCv(cv_addr + 1u, _, A<std::function<void(uint8_t)>>()));

  auto other_cv_packet{
    make_cv_access_long_verify_packet(_addrs.primary, cv_addr + 1u)};
  Receive(other_cv_packet)->LeaveCutout()->Execute()->Receive(other_cv_packet);

  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(acks))).Times(1);
  _mock.biDiChannel2();
}
