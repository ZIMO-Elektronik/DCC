#include <gtest/gtest.h>
#include <dcc/dcc.hpp>

TEST(instruction, decode_instruction_short_address) {
  auto packet{dcc::make_cv_access_long_verify_packet(3u, 0u)};
  EXPECT_EQ(dcc::decode_instruction(packet), dcc::Instruction::CvLong);
}

TEST(instruction, decode_instruction_long_address) {
  {
    auto packet{dcc::make_cv_access_long_verify_packet(1022u, 0u)};
    EXPECT_EQ(dcc::decode_instruction(packet), dcc::Instruction::CvLong);
  }

  {
    auto packet{dcc::make_cv_access_long_verify_packet(1337u, 0u)};
    EXPECT_EQ(dcc::decode_instruction(packet), dcc::Instruction::CvLong);
  }
}
