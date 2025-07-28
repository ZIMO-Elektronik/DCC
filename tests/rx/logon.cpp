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

// Known CID and session ID <=4 skips logon
TEST_F(RxTest, logon_with_known_cid_and_session_id_le_4) {
  Logon();

  // Execute commands to address 1000
  EXPECT_CALL(_mock, writeCv(_, _)).Times(7);
  EXPECT_CALL(_mock, direction(_addrs.primary.value, false));
  EXPECT_CALL(_mock, speed(_addrs.primary.value, _));
  ReceiveAndExecute(make_advanced_operations_speed_packet(_addrs.logon, 0u));
}

// Known CID and session ID >4 forces logon
TEST_F(RxTest, logon_with_known_cid_and_session_id_gt_4) {
  EXPECT_CALL(_mock, transmitBiDi(_)).Times(AtLeast(3 * 2));

  // Enable
  // Send multiple times to make sure implementation does not skip logon upon
  // receiving session ID multiple times in a row. That used to be a bug.
  for (auto i{0uz}; i < 10uz; ++i) {
    Receive(
      make_logon_enable_packet(dcc::AddressGroup::Now, _cid, _session_id + 5u));
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
TEST_F(RxTest, no_id15_datagram_after_logon_select) {
  EXPECT_CALL(_mock, transmitBiDi(_)).Times(2 * 2);

  // Enable
  Receive(make_logon_enable_packet(
    dcc::AddressGroup::Now, _cid + 1u, RandomInterval<uint8_t>(0u, 255u)));
  BiDi();

  // Select
  Receive(dcc::make_logon_select_packet(DCC_MANUFACTURER_ID, _did));
  BiDi();

  // Enable (again)
  Receive(make_logon_enable_packet(
    dcc::AddressGroup::Now, _cid + 1u, RandomInterval<uint8_t>(0u, 255u)));
  BiDi();
}
