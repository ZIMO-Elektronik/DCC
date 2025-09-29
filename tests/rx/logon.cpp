#include "rx_test.hpp"

// Known CID skips logon
TEST_F(RxTest, skip_logon_with_known_cid) {
  Logon();

  // Execute commands to logon address
  EXPECT_CALL(_mock, writeCv(_, _)).Times(7);
  EXPECT_CALL(_mock, direction(_addrs.primary.value, false));
  EXPECT_CALL(_mock, speed(_addrs.primary.value, _));
  ReceiveAndExecute(
    dcc::make_advanced_operations_speed_packet(_addrs.logon, 0u));
}

// Unknown CID forces logon
TEST_F(RxTest, logon_with_unknown_cid_basic_loco) {
  EXPECT_CALL(_mock, transmitBiDi(_)).Times(3 * 2);

  // Enable
  Receive(make_logon_enable_packet(
    dcc::AddressGroup::Now, _cid + 1u, RandomInterval<uint8_t>(0u, 255u)));
  BiDi();

  // Select
  Receive(dcc::make_logon_select_packet(DCC_MANUFACTURER_ID, _did));
  BiDi();

  // Assign address 42
  _addrs.logon = {.value = 42u, .type = dcc::Address::BasicLoco};
  Receive(
    dcc::make_logon_assign_packet(DCC_MANUFACTURER_ID, _did, _addrs.logon));
  BiDi();

  // Execute commands to address 42
  EXPECT_CALL(_mock, writeCv(_, _)).Times(7);
  EXPECT_CALL(_mock, direction(_addrs.primary.value, false));
  EXPECT_CALL(_mock, speed(_addrs.primary.value, _));
  ReceiveAndExecute(
    dcc::make_advanced_operations_speed_packet(_addrs.logon, 0u));
}

// Unknown CID forces logon
TEST_F(RxTest, logon_with_unknown_cid_extended_loco) {
  EXPECT_CALL(_mock, transmitBiDi(_)).Times(3 * 2);

  // Enable
  Receive(make_logon_enable_packet(
    dcc::AddressGroup::Now, _cid + 1u, RandomInterval<uint8_t>(0u, 255u)));
  BiDi();

  // Select
  Receive(dcc::make_logon_select_packet(DCC_MANUFACTURER_ID, _did));
  BiDi();

  // Assign address 1001
  _addrs.logon = {.value = 1001u, .type = dcc::Address::ExtendedLoco};
  Receive(
    dcc::make_logon_assign_packet(DCC_MANUFACTURER_ID, _did, _addrs.logon));
  BiDi();

  // Execute commands to address 1001
  EXPECT_CALL(_mock, writeCv(_, _)).Times(7);
  EXPECT_CALL(_mock, direction(_addrs.primary.value, false));
  EXPECT_CALL(_mock, speed(_addrs.primary.value, _));
  ReceiveAndExecute(
    dcc::make_advanced_operations_speed_packet(_addrs.logon, 0u));
}

// Known CID and unknown SID doesn't skip logon
TEST_F(RxTest, no_logon_with_known_cid_and_unknown_sid) {
  EXPECT_CALL(_mock, readCv(_)).Times(0);

  // Enable
  Receive(
    dcc::make_logon_enable_packet(dcc::AddressGroup::Now, _cid, _sid + 1u));

  // Execute commands to logon address
  EXPECT_CALL(_mock, writeCv(_, _)).Times(0);
  EXPECT_CALL(_mock, direction(_addrs.primary.value, false)).Times(0);
  EXPECT_CALL(_mock, speed(_addrs.primary.value, _)).Times(0);
  ReceiveAndExecute(make_advanced_operations_speed_packet(_addrs.logon, 0u));
}

// Known CID and SID incremented by >1 forces relogon
TEST_F(RxTest, force_new_logon_with_known_cid_and_sid_plus_2) {
  Logon();

  // Execute commands to logon address
  EXPECT_CALL(_mock, writeCv(_, _)).Times(7);
  EXPECT_CALL(_mock, direction(_addrs.primary.value, false));
  EXPECT_CALL(_mock, speed(_addrs.primary.value, _));
  ReceiveAndExecute(
    dcc::make_advanced_operations_speed_packet(_addrs.logon, 0u));

  EXPECT_CALL(_mock, transmitBiDi(_)).Times(3 * 2);

  // Enable
  Receive(make_logon_enable_packet(dcc::AddressGroup::Now, _cid, _sid + 2u));
  BiDi();

  // Select
  Receive(dcc::make_logon_select_packet(DCC_MANUFACTURER_ID, _did));
  BiDi();

  // Assign address 1001
  _addrs.logon = {.value = 1001u, .type = dcc::Address::ExtendedLoco};
  Receive(
    dcc::make_logon_assign_packet(DCC_MANUFACTURER_ID, _did, _addrs.logon));
  BiDi();

  // Execute commands to address 1001
  EXPECT_CALL(_mock, writeCv(_, _)).Times(7);
  EXPECT_CALL(_mock, direction(_addrs.primary.value, false));
  EXPECT_CALL(_mock, speed(_addrs.primary.value, _));
  ReceiveAndExecute(
    dcc::make_advanced_operations_speed_packet(_addrs.logon, 0u));
}

// LOGON_SELECT disables LOGON_ENABLE (and ID15 datagram)
TEST_F(
  RxTest,
  no_id15_datagram_after_logon_select_as_long_as_cid_and_sid_stay_the_same) {
  EXPECT_CALL(_mock, readCv(DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 0u))
    .WillRepeatedly(Return(_cvs[DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 0uz]));
  EXPECT_CALL(_mock, readCv(DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 1u))
    .WillRepeatedly(Return(_cvs[DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 1uz]));

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
