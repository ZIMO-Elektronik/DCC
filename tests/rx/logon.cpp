#include "rx_test.hpp"

TEST_F(RxTest, logon_with_new_cid_and_session_id) {
  EXPECT_CALL(_mock, transmitBiDi(_)).Times(3 * 2);

  // Enable
  Receive(dcc::make_logon_enable_packet(dcc::AddressGroup::Now, 0u, 0u));
  Cutout();

  // Select
  Receive(dcc::make_logon_select_packet(dcc::zimo_id, _did));
  Cutout();

  // Assign address 1000
  Receive(
    dcc::make_logon_assign_packet(dcc::zimo_id, _did, (0b11 << 14u) | 1000u));
  Cutout();

  // Execute commands to address 1000
  EXPECT_CALL(_mock, direction(3u, false));
  EXPECT_CALL(_mock, speed(3u, _));
  Receive(dcc::make_advanced_operations_speed_packet(1000u, 0u));
  Execute();
}
