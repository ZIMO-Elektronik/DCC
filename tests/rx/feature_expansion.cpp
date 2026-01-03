#include "rx_test.hpp"

TEST_F(RxTest, feature_expansion_f20_f13) {
  auto state{RandomInterval<uint8_t>(0x00u, 0xFFu)};
  EXPECT_CALL(_mock,
              function(_addrs.primary.value,
                       0xFFu << 13u,
                       static_cast<uint32_t>(state << 13u)));
  ReceiveAndExecute(make_f13_f20_packet(_addrs.primary, state));
}

TEST_F(RxTest, feature_expansion_f20_f13_wrong_packet_length) {
  auto state{RandomInterval<uint8_t>(0x00u, 0xFFu)};
  EXPECT_CALL(_mock,
              function(_addrs.primary.value,
                       0xFFu << 13u,
                       static_cast<uint32_t>(state << 13u)))
    .Times(0);
  ReceiveAndExecute(
    TinkerWithPacketLength(make_f13_f20_packet(_addrs.primary, state)));
}

TEST_F(RxTest, feature_expansion_f28_f21) {
  auto state{RandomInterval<uint8_t>(0x00u, 0xFFu)};
  EXPECT_CALL(_mock,
              function(_addrs.primary.value,
                       0xFFu << 21u,
                       static_cast<uint32_t>(state << 21u)));
  ReceiveAndExecute(make_f21_f28_packet(_addrs.primary, state));
}
