#include <gtest/gtest.h>
#include <dcc/dcc.hpp>

TEST(instruction, decode_instruction_short_address) {
  auto packet{dcc::make_consist_control_packet(3u, 0u)};
  EXPECT_EQ(dcc::decode_instruction(packet), dcc::Instruction::ConsistControl);
}

TEST(instruction, decode_instruction_long_address) {
  {
    auto packet{dcc::make_function_group_f4_f0_packet(1022u, 0u)};
    EXPECT_EQ(dcc::decode_instruction(packet), dcc::Instruction::FunctionGroup);
  }

  {
    auto packet{dcc::make_consist_control_packet(1337u, 0u)};
    EXPECT_EQ(dcc::decode_instruction(packet),
              dcc::Instruction::ConsistControl);
  }
}
