#include "rx_test.hpp"

TEST_F(RxTest, consist_shall_not_act_on_cv_manipulation) {
  _cvs[19uz - 1uz] = static_cast<uint8_t>(_addrs.consist);
  SetUp();

  // Don't write any CV which might trigger config (e.g. 1, 28, ...)!
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto byte{RandomInterval<uint8_t>(0u, 255u)};

  EXPECT_CALL(_mock, writeCv(cv_addr, byte)).Times(0);
  ReceiveAndExecuteTwoIdenticalCvLongWritePackets(
    _addrs.consist, cv_addr, byte);
}

TEST_F(RxTest, consist_direction) {
  _cvs[19uz - 1uz] = static_cast<uint8_t>(_addrs.consist);
  _cvs[29uz - 1uz] = static_cast<uint8_t>(_cvs[29uz - 1uz]);
  SetUp();

  EXPECT_CALL(_mock, direction(_addrs.consist.value, dcc::Forward));
  EXPECT_CALL(_mock, speed(_addrs.consist.value, _));
  Receive(dcc::make_advanced_operations_speed_packet(_addrs.consist,
                                                     dcc::Forward << 7u | 10u));
  Execute();
}

TEST_F(RxTest, consist_direction_reversed) {
  _cvs[19uz - 1uz] = static_cast<uint8_t>(1u << 7u | _addrs.consist);
  _cvs[29uz - 1uz] = static_cast<uint8_t>(_cvs[29uz - 1uz]);
  SetUp();

  EXPECT_CALL(_mock, direction(_addrs.consist.value, dcc::Backward));
  EXPECT_CALL(_mock, speed(_addrs.consist.value, _));
  Receive(dcc::make_advanced_operations_speed_packet(_addrs.consist,
                                                     dcc::Forward << 7u | 10u));
  Execute();
}

TEST_F(RxTest, consist_only_primary_direction_reversed) {
  _cvs[19uz - 1uz] = static_cast<uint8_t>(_addrs.consist);
  _cvs[29uz - 1uz] = static_cast<uint8_t>(_cvs[29uz - 1uz] | 0b1u);
  SetUp();

  EXPECT_CALL(_mock, direction(_addrs.consist.value, dcc::Backward));
  EXPECT_CALL(_mock, speed(_addrs.consist.value, _));
  Receive(dcc::make_advanced_operations_speed_packet(_addrs.consist,
                                                     dcc::Forward << 7u | 10u));
  Execute();
}

TEST_F(RxTest, consist_and_primary_direction_reversed) {
  _cvs[19uz - 1uz] = static_cast<uint8_t>(1u << 7u | _addrs.consist);
  _cvs[29uz - 1uz] = static_cast<uint8_t>(_cvs[29uz - 1uz] | 0b1u);
  SetUp();

  EXPECT_CALL(_mock, direction(_addrs.consist.value, dcc::Forward));
  EXPECT_CALL(_mock, speed(_addrs.consist.value, _));
  Receive(dcc::make_advanced_operations_speed_packet(_addrs.consist,
                                                     dcc::Forward << 7u | 10u));
  Execute();
}
