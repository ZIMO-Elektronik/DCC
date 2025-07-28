#include "rx_test.hpp"

TEST_F(RxTest, feature_expansion_f20_f13) {
  auto state{RandomInterval<uint8_t>(0x00u, 0xFFu)};
  EXPECT_CALL(_mock,
              function(_addrs.primary.value,
                       0xFFu << 13u,
                       static_cast<uint32_t>(state << 13u)));
  Receive(make_feature_expansion_f20_f13_packet(_addrs.primary, state));
  Execute();
}

TEST_F(RxTest, feature_expansion_f20_f13_wrong_packet_length) {
  auto state{RandomInterval<uint8_t>(0x00u, 0xFFu)};
  EXPECT_CALL(_mock,
              function(_addrs.primary.value,
                       0xFFu << 13u,
                       static_cast<uint32_t>(state << 13u)))
    .Times(0);
  Receive(TinkerWithPacketLength(
    make_feature_expansion_f20_f13_packet(_addrs.primary, state)));
  Execute();
}

TEST_F(RxTest, feature_expansion_f28_f21) {
  auto state{RandomInterval<uint8_t>(0x00u, 0xFFu)};
  EXPECT_CALL(_mock,
              function(_addrs.primary.value,
                       0xFFu << 21u,
                       static_cast<uint32_t>(state << 21u)));
  Receive(make_feature_expansion_f28_f21_packet(_addrs.primary, state));
  Execute();
}
