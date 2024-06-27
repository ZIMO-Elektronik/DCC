#include "rx_test.hpp"

// New CID forces new logon
TEST_F(RxTest, logon_with_new_cid) {
  EXPECT_CALL(_mock, transmitBiDi(_)).Times(3 * 2);

  // Enable
  Receive(dcc::make_logon_enable_packet(
    dcc::AddressGroup::Now, _cid + 1u, static_cast<uint8_t>(rand())));
  Cutout();

  // Select
  Receive(dcc::make_logon_select_packet(dcc::zimo_id, _did));
  Cutout();

  // Assign address 1000
  Receive(
    dcc::make_logon_assign_packet(dcc::zimo_id, _did, (0b11 << 14u) | 1000u));
  Cutout();

  // Execute commands to address 1000
  EXPECT_CALL(_mock, writeCv(_, _)).Times(7);
  EXPECT_CALL(_mock, direction(_addrs.primary.value, false));
  EXPECT_CALL(_mock, speed(_addrs.primary.value, _));
  Receive(dcc::make_advanced_operations_speed_packet(1000u, 0u));
  Execute();
}

// Known CID and session ID <=4 skips logon
TEST_F(RxTest, logon_with_known_cid_and_session_id_le_4) {
  Logon();

  // Execute commands to address 1000
  EXPECT_CALL(_mock, writeCv(_, _)).Times(7);
  EXPECT_CALL(_mock, direction(_addrs.primary.value, false));
  EXPECT_CALL(_mock, speed(_addrs.primary.value, _));
  Receive(dcc::make_advanced_operations_speed_packet(_addrs.logon, 0u));
  Execute();
}

// Known CID and session ID >4 forces logon
TEST_F(RxTest, logon_with_known_cid_and_session_id_gt_4) {
  EXPECT_CALL(_mock, transmitBiDi(_)).Times(AtLeast(3 * 2));

  // Enable
  // Send multiple times to make sure implementation does not skip logon upon
  // receiving session ID multiple times in a row. That used to be a bug.
  for (auto i{0uz}; i < 10uz; ++i) {
    Receive(dcc::make_logon_enable_packet(
      dcc::AddressGroup::Now, _cid, _session_id + 5u));
    Cutout();
  }

  // Select
  Receive(dcc::make_logon_select_packet(dcc::zimo_id, _did));
  Cutout();

  // Assign address 1000
  Receive(
    dcc::make_logon_assign_packet(dcc::zimo_id, _did, (0b11 << 14u) | 1000u));
  Cutout();

  // Execute commands to address 1000
  EXPECT_CALL(_mock, writeCv(_, _)).Times(7);
  EXPECT_CALL(_mock, direction(_addrs.primary.value, false));
  EXPECT_CALL(_mock, speed(_addrs.primary.value, _));
  Receive(dcc::make_advanced_operations_speed_packet(1000u, 0u));
  Execute();
}
