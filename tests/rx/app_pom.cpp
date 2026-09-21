#include "rx_test.hpp"

using namespace dcc::bidi;

TEST_F(RxTest, app_pom) {
  auto cv_addr{RandomInterval<uint8_t>(0u, 255u)};
  auto byte{RandomInterval<uint8_t>(0u, 255u)};
  auto packet{make_cv_access_long_verify_packet(_addrs.primary, cv_addr)};

  EXPECT_CALL(_mock, readCv(cv_addr, _, A<std::function<void(uint8_t)>>()))
    .WillOnce(InvokeArgument<2uz>(byte));
  Receive(packet)->LeaveCutout()->Execute()->EnterCutout();

  auto datagram{make_app_pom_datagram(byte)};
  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  _mock.biDiChannel2();
}

// https://github.com/ZIMO-Elektronik/DCC/issues/20
TEST_F(RxTest, app_pom_responds_to_address_that_sent_request) {
  // Change address by writing CV1
  auto cv_addr{1u - 1u};
  auto byte{static_cast<uint8_t>(_addrs.primary + 1u)};
  auto packet{make_cv_access_long_write_packet(_addrs.primary, cv_addr, byte)};

  EXPECT_CALL(_mock, writeCv(cv_addr, byte, A<std::function<void(uint8_t)>>()))
    .WillOnce(InvokeArgument<2uz>(byte));
  EXPECT_CALL(_mock, readCv(_)).BASIC_ADDRESS_READ_CV_INIT_SEQUENCE();
  ReceiveAndExecuteTwice(packet);

  // Packet to the new address, no POM
  auto datagram{make_app_pom_datagram(byte)};
  Receive(dcc::make_f0_f4_packet(_addrs.primary + 1u, 0u));
  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(0);
  _mock.biDiChannel2();

  // Packet to the old address, expect POM
  Receive(packet);
  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  _mock.biDiChannel2();
}

TEST_F(RxTest, app_pom_respond_to_cv_access_long_write_bit_operations_mode) {
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
  EnterCutout();
  _mock.biDiChannel2();
}

TEST_F(RxTest, app_pom_respond_to_cv_access_short_cv17_18) {
  dcc::Address new_extended_address{.value = 1337u,
                                    .type = dcc::Address::ExtendedLoco};
  encode_address(new_extended_address, &_cvs[17uz - 1uz]);

  // CV17, CV18 and CV29:5 will be written
  EXPECT_CALL(_mock, writeCv(17u - 1u, _cvs[17uz - 1uz]))
    .WillOnce(Return(_cvs[17uz - 1uz]));
  EXPECT_CALL(_mock, writeCv(18u - 1u, _cvs[18uz - 1uz]))
    .WillOnce(Return(_cvs[18uz - 1uz]));
  EXPECT_CALL(_mock,
              writeCv(29u - 1u, true, 5u, A<std::function<void(bool)>>()));
  // ... then init sequence will be read
  EXPECT_CALL(_mock, readCv(_))
    .EXTENDED_ADDRESS_READ_CV_INIT_SEQUENCE_CHANGE_CV29(_cvs[29uz - 1uz] |
                                                        ztl::mask<5u>);

  // Change address
  for (auto i{0uz}; i < 2uz; ++i)
    ReceiveAndExecute(make_cv_access_short_write_packet(
      _addrs.primary, 0b0100u, _cvs[17uz - 1uz], _cvs[18uz - 1uz]));

  // ... and generate a special ID0-ID0 response
  auto cv17_datagram{make_app_pom_datagram(_cvs[17uz - 1uz])};
  auto cv18_datagram{make_app_pom_datagram(_cvs[18uz - 1uz])};
  Datagram<datagram_size<Bits::_24>> datagram{cv17_datagram[0uz],
                                              cv17_datagram[1uz],
                                              cv18_datagram[0uz],
                                              cv18_datagram[1uz]};
  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  EnterCutout();
  _mock.biDiChannel2();
}

TEST_F(RxTest, app_pom_respond_to_cv_access_short_cv31_32) {
  _cvs[31uz - 1uz] = 145u;
  _cvs[32uz - 1uz] = 0u;
  auto packet{dcc::make_cv_access_short_write_packet(
    _addrs.primary, 0b0101u, _cvs[31uz - 1uz], _cvs[32uz - 1uz])};

  // Write CV31 and CV32
  EXPECT_CALL(_mock, writeCv(31u - 1u, _cvs[31uz - 1uz]))
    .WillOnce(Return(_cvs[31uz - 1uz]));
  EXPECT_CALL(_mock, writeCv(32u - 1u, _cvs[32uz - 1uz]))
    .WillOnce(Return(_cvs[32uz - 1uz]));
  ReceiveAndExecuteTwice(packet);

  // ... and generate a special ID0-ID0 response
  auto cv31_datagram{make_app_pom_datagram(_cvs[31uz - 1uz])};
  auto cv32_datagram{make_app_pom_datagram(_cvs[32uz - 1uz])};
  Datagram<datagram_size<Bits::_24>> datagram{cv31_datagram[0uz],
                                              cv31_datagram[1uz],
                                              cv32_datagram[0uz],
                                              cv32_datagram[1uz]};
  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  EnterCutout();
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
  Receive(packet)->LeaveCutout()->Execute()->EnterCutout();

  EXPECT_CALL(_mock, transmitBiDi(_)).Times(0);
  _mock.biDiChannel2();
}

TEST_F(RxTest, app_pom_responds_to_all_loco_packets) {
  auto cv_addr{RandomInterval<uint8_t>(0u, 255u)};
  auto byte{RandomInterval<uint8_t>(0u, 255u)};
  auto packet{make_cv_access_long_verify_packet(_addrs.primary, cv_addr)};

  EXPECT_CALL(_mock, readCv(cv_addr, _, A<std::function<void(uint8_t)>>()))
    .WillOnce(InvokeArgument<2uz>(byte));
  Receive(packet)->LeaveCutout()->Execute();

  // Any loco packet to this address can trigger a POM response
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
  Receive(other_cv_packet)->LeaveCutout()->Execute()->EnterCutout();

  auto datagram{make_app_pom_datagram(byte)};
  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(0);
  _mock.biDiChannel2();
}
