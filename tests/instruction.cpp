#include <gtest/gtest.h>
#include <dcc/dcc.hpp>

TEST(instruction, decode_decoder_control) {
  uint8_t instr{};
  instr = 0b0000'0000u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::DecoderControl);
  instr = 0b0000'0001u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::DecoderControl);
  instr = 0b0000'0010u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::DecoderControl);
  instr = 0b0000'0011u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::DecoderControl);
  instr = 0b0000'0100u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0000'0101u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0000'0110u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0000'0111u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0000'1000u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0000'1001u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0000'1010u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::DecoderControl);
  instr = 0b0000'1011u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::DecoderControl);
  instr = 0b0000'1100u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0000'1101u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0000'1110u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0000'1111u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::DecoderControl);
}

TEST(instruction, decode_consist_control) {
  uint8_t instr{};
  instr = 0b0001'0000u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0001'0001u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0001'0010u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::ConsistControl);
  instr = 0b0001'0011u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::ConsistControl);
  instr = 0b0001'0100u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0001'0101u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0001'0110u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0001'0111u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0001'1000u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0001'1001u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0001'1010u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0001'1011u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0001'1100u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0001'1101u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0001'1110u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0001'1111u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
}

TEST(instruction, decode_advanced_operations) {
  uint8_t instr{};
  instr = 0b0010'0000u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0010'0001u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0010'0010u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0010'0011u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0010'0100u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0010'0101u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0010'0110u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0010'0111u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0010'1000u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0010'1001u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0010'1010u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0010'1011u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0010'1100u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0010'1101u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0010'1110u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0010'1111u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0011'0000u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0011'0001u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0011'0010u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0011'0011u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0011'0100u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0011'0101u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0011'0110u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0011'0111u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0011'1000u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0011'1001u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0011'1010u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0011'1011u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b0011'1100u;
  EXPECT_EQ(dcc::decode_instruction(&instr),
            dcc::Instruction::AdvancedOperations);
  instr = 0b0011'1101u;
  EXPECT_EQ(dcc::decode_instruction(&instr),
            dcc::Instruction::AdvancedOperations);
  instr = 0b0011'1110u;
  EXPECT_EQ(dcc::decode_instruction(&instr),
            dcc::Instruction::AdvancedOperations);
  instr = 0b0011'1111u;
  EXPECT_EQ(dcc::decode_instruction(&instr),
            dcc::Instruction::AdvancedOperations);
}

TEST(instruction, decode_speed_direction) {
  for (uint8_t instr{0b0100'0000u}; instr <= 0b0111'1111u; ++instr)
    EXPECT_EQ(dcc::decode_instruction(&instr),
              dcc::Instruction::SpeedDirection);
}

TEST(instruction, decode_function_group) {
  for (uint8_t instr{0b1000'0000u}; instr <= 0b1011'1111u; ++instr)
    EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::FunctionGroup);
}

TEST(instruction, decode_feature_expansion) {
  uint8_t instr{};
  instr = 0b1100'0000u;
  EXPECT_EQ(dcc::decode_instruction(&instr),
            dcc::Instruction::FeatureExpansion);
  instr = 0b1100'0001u;
  EXPECT_EQ(dcc::decode_instruction(&instr),
            dcc::Instruction::FeatureExpansion);
  instr = 0b1100'0010u;
  EXPECT_EQ(dcc::decode_instruction(&instr),
            dcc::Instruction::FeatureExpansion);
  instr = 0b1100'0011u;
  EXPECT_EQ(dcc::decode_instruction(&instr),
            dcc::Instruction::FeatureExpansion);
  instr = 0b1100'0100u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1100'0101u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1100'0110u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1100'0111u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1100'1000u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1100'1001u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1100'1010u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1100'1011u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1100'1100u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1100'1101u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1100'1110u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1100'1111u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1101'0000u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1101'0001u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1101'0010u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1101'0011u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1101'0100u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1101'0101u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1101'0110u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1101'0111u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1101'1000u;
  EXPECT_EQ(dcc::decode_instruction(&instr),
            dcc::Instruction::FeatureExpansion);
  instr = 0b1101'1001u;
  EXPECT_EQ(dcc::decode_instruction(&instr),
            dcc::Instruction::FeatureExpansion);
  instr = 0b1101'1010u;
  EXPECT_EQ(dcc::decode_instruction(&instr),
            dcc::Instruction::FeatureExpansion);
  instr = 0b1101'1011u;
  EXPECT_EQ(dcc::decode_instruction(&instr),
            dcc::Instruction::FeatureExpansion);
  instr = 0b1101'1100u;
  EXPECT_EQ(dcc::decode_instruction(&instr),
            dcc::Instruction::FeatureExpansion);
  instr = 0b1101'1101u;
  EXPECT_EQ(dcc::decode_instruction(&instr),
            dcc::Instruction::FeatureExpansion);
  instr = 0b1101'1110u;
  EXPECT_EQ(dcc::decode_instruction(&instr),
            dcc::Instruction::FeatureExpansion);
  instr = 0b1101'1111u;
  EXPECT_EQ(dcc::decode_instruction(&instr),
            dcc::Instruction::FeatureExpansion);
}

TEST(instruction, decode_cv_access) {
  uint8_t instr{};
  instr = 0b1110'0000u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1110'0001u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1110'0010u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1110'0011u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1110'0100u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::CvAccess);
  instr = 0b1110'0101u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::CvAccess);
  instr = 0b1110'0110u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::CvAccess);
  instr = 0b1110'0111u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::CvAccess);
  instr = 0b1110'1000u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::CvAccess);
  instr = 0b1110'1001u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::CvAccess);
  instr = 0b1110'1010u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::CvAccess);
  instr = 0b1110'1011u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::CvAccess);
  instr = 0b1110'1100u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::CvAccess);
  instr = 0b1110'1101u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::CvAccess);
  instr = 0b1110'1110u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::CvAccess);
  instr = 0b1110'1111u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::CvAccess);

  instr = 0b1111'0000u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1111'0001u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1111'0010u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::CvAccess);
  instr = 0b1111'0011u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::CvAccess);
  instr = 0b1111'0100u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::CvAccess);
  instr = 0b1111'0101u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::CvAccess);
  instr = 0b1111'0110u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::CvAccess);
  instr = 0b1111'0111u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1111'1000u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1111'1001u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::CvAccess);
  instr = 0b1111'1010u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1111'1011u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1111'1100u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1111'1101u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
  instr = 0b1111'1110u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::Logon);
  instr = 0b1111'1111u;
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::UnknownService);
}

TEST(instruction, decode_logon) {
  uint8_t instr{0b1111'1110u};
  EXPECT_EQ(dcc::decode_instruction(&instr), dcc::Instruction::Logon);
}

TEST(instruction, decode_instruction_short_address) {
  auto packet{dcc::make_set_consist_address_packet(3u, 0u)};
  EXPECT_EQ(dcc::decode_instruction(packet), dcc::Instruction::ConsistControl);
}

TEST(instruction, decode_instruction_long_address) {
  {
    auto packet{dcc::make_f0_f4_packet(1022u, 0u)};
    EXPECT_EQ(dcc::decode_instruction(packet), dcc::Instruction::FunctionGroup);
  }

  {
    auto packet{dcc::make_set_consist_address_packet(1337u, 0u)};
    EXPECT_EQ(dcc::decode_instruction(packet),
              dcc::Instruction::ConsistControl);
  }
}
