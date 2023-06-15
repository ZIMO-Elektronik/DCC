#include "rx_test.hpp"

// 126 speed steps command forward
TEST_F(RxTest, _126_speed_steps_fwd) {
  Expectation read_cv{
    EXPECT_CALL(mock_, readCv(_)).WillOnce(Return(cvs_[29uz - 1uz]))};
  Expectation dir{EXPECT_CALL(mock_, direction(3u, 1))};
  Expectation notch{EXPECT_CALL(mock_, notch(3u, _))};
  Receive(dcc::make_advanced_operations_speed_packet(3u, 1, 10u));
  Execute();
}

// 126 speed steps command backward
TEST_F(RxTest, _126_speed_steps_bwd) {
  {
    Expectation read_cv{
      EXPECT_CALL(mock_, readCv(_)).WillOnce(Return(cvs_[29uz - 1uz]))};
    Expectation dir{EXPECT_CALL(mock_, direction(3u, -1))};
    Expectation notch{EXPECT_CALL(mock_, notch(3u, _))};
    Receive(dcc::make_advanced_operations_speed_packet(3u, -1, 10u));
    Execute();
  }

  // dir=0 is accepted as backwards as well
  {
    Expectation read_cv{
      EXPECT_CALL(mock_, readCv(_)).WillOnce(Return(cvs_[29uz - 1uz]))};
    Expectation dir{EXPECT_CALL(mock_, direction(3u, -1))};
    Expectation notch{EXPECT_CALL(mock_, notch(3u, _))};
    Receive(dcc::make_advanced_operations_speed_packet(3u, 0, 10u));
    Execute();
  }
}
