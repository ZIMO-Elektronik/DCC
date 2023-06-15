#include "rx_test.hpp"

TEST_F(RxTest, feature_expansion_f20_f13) {
  auto state{RandomInterval(0x00u, 0xFFu)};
  Expectation function{
    EXPECT_CALL(mock_, function(3u, 0xFFu << 13u, state << 13u))};
  Receive(dcc::make_feature_expansion_f20_f13_packet(3u, state));
  Execute();
}

TEST_F(RxTest, feature_expansion_f28_f21) {
  auto state{RandomInterval(0x00u, 0xFFu)};
  Expectation function{
    EXPECT_CALL(mock_, function(3u, 0xFFu << 21u, state << 21u))};
  Receive(dcc::make_feature_expansion_f28_f21_packet(3u, state));
  Execute();
}