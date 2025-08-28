#include "rx_test.hpp"

// New CID forces new logon
TEST_F(RxTest, logon_with_new_cid) {
  EXPECT_CALL(_mock, transmitBiDi(_)).Times(3 * 2);

  // Enable
  Receive(make_logon_enable_packet(
    dcc::AddressGroup::Now, _cid + 1u, RandomInterval<uint8_t>(0u, 255u)));
  BiDi();

  // Select
  Receive(dcc::make_logon_select_packet(DCC_MANUFACTURER_ID, _did));
  BiDi();

  // Assign address 1000
  Receive(dcc::make_logon_assign_packet(
    DCC_MANUFACTURER_ID, _did, (0b11 << 14u) | 1000u));
  BiDi();

  // Execute commands to address 1000
  EXPECT_CALL(_mock, writeCv(_, _)).Times(7);
  EXPECT_CALL(_mock, direction(_addrs.primary.value, false));
  EXPECT_CALL(_mock, speed(_addrs.primary.value, _));
  ReceiveAndExecute(dcc::make_advanced_operations_speed_packet(1000u, 0u));
}

// Known CID and session ID skips logon
TEST_F(RxTest, logon_with_known_cid_and_sid) {
  Logon();

  // Execute commands to address 1000
  EXPECT_CALL(_mock, writeCv(_, _)).Times(7);
  EXPECT_CALL(_mock, direction(_addrs.primary.value, false));
  EXPECT_CALL(_mock, speed(_addrs.primary.value, _));
  ReceiveAndExecute(make_advanced_operations_speed_packet(_addrs.logon, 0u));
}

// Known CID and session ID incremented by 1 skips logon
TEST_F(RxTest, logon_with_known_cid_and_sid_plus_1) {
  EXPECT_CALL(_mock, readCv(_))
    .WillOnce(Return(_cvs[65300uz - 1uz]))
    .WillOnce(Return(_cvs[65301uz - 1uz]));

  // Enable
  Receive(
    dcc::make_logon_enable_packet(dcc::AddressGroup::Now, _cid, _sid + 1u));

  // Execute commands to address 1000
  EXPECT_CALL(_mock, writeCv(_, _)).Times(7);
  EXPECT_CALL(_mock, direction(_addrs.primary.value, false));
  EXPECT_CALL(_mock, speed(_addrs.primary.value, _));
  ReceiveAndExecute(make_advanced_operations_speed_packet(_addrs.logon, 0u));
}

// Known CID and session ID incremented by >1 forces logon
TEST_F(RxTest, logon_with_known_cid_and_sid_plus_2) {
  EXPECT_CALL(_mock, transmitBiDi(_)).Times(AtLeast(3 * 2));

  // Enable
  // Send multiple times to make sure implementation does not skip logon upon
  // receiving session ID multiple times in a row. That used to be a bug.
  for (auto i{0uz}; i < 10uz; ++i) {
    Receive(make_logon_enable_packet(dcc::AddressGroup::Now, _cid, _sid + 2u));
    BiDi();
  }

  // Select
  Receive(dcc::make_logon_select_packet(DCC_MANUFACTURER_ID, _did));
  BiDi();

  // Assign address 1000
  Receive(dcc::make_logon_assign_packet(
    DCC_MANUFACTURER_ID, _did, (0b11 << 14u) | 1000u));
  BiDi();

  // Execute commands to address 1000
  EXPECT_CALL(_mock, writeCv(_, _)).Times(7);
  EXPECT_CALL(_mock, direction(_addrs.primary.value, false));
  EXPECT_CALL(_mock, speed(_addrs.primary.value, _));
  ReceiveAndExecute(dcc::make_advanced_operations_speed_packet(1000u, 0u));
}

// LOGON_SELECT disables LOGON_ENABLE (and ID15 datagram)
TEST_F(
  RxTest,
  no_id15_datagram_after_logon_select_as_long_as_cid_and_sid_stay_the_same) {
  EXPECT_CALL(_mock, readCv(65300u - 1u))
    .WillRepeatedly(Return(_cvs[65300uz - 1uz]));
  EXPECT_CALL(_mock, readCv(65301u - 1u))
    .WillRepeatedly(Return(_cvs[65301uz - 1uz]));

  EXPECT_CALL(_mock, transmitBiDi(_)).Times(3 * 2);

  // Enable, expect ID15
  Receive(make_logon_enable_packet(dcc::AddressGroup::Now, _cid + 1u, _sid));
  BiDi();

  // Select, expect ID13
  Receive(dcc::make_logon_select_packet(DCC_MANUFACTURER_ID, _did));
  BiDi();

  // Enable (again)
  Receive(make_logon_enable_packet(dcc::AddressGroup::Now, _cid + 1u, _sid));
  BiDi();

  // Enable (again with new SID), expect ID15
  Receive(
    make_logon_enable_packet(dcc::AddressGroup::Now, _cid + 1u, _sid + 1u));
  BiDi();
}
