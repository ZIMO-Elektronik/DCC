#include <gtest/gtest.h>
#include <dcc/dcc.hpp>

using dcc::scale_speed;

TEST(receive_scale_speed, zero) {
  EXPECT_EQ(scale_speed<14>(0), 0);
  EXPECT_EQ(scale_speed<28>(0), 0);
  EXPECT_EQ(scale_speed<126>(0), 0);
}

TEST(receive_scale_speed, one) {
  EXPECT_EQ(scale_speed<14>(1), scale_speed<126>(1));
  EXPECT_EQ(scale_speed<28>(1), scale_speed<126>(1));
}

TEST(receive_scale_speed, two) {
  EXPECT_EQ(scale_speed<14>(2), 21);
  EXPECT_EQ(scale_speed<28>(2), 11);
  EXPECT_EQ(scale_speed<126>(2), 4);
}

TEST(receive_scale_speed, three) {
  EXPECT_EQ(scale_speed<14>(3), 40);
  EXPECT_EQ(scale_speed<28>(3), 20);
  EXPECT_EQ(scale_speed<126>(3), 6);
}

TEST(receive_scale_speed, max) {
  EXPECT_EQ(scale_speed<14>(14), 255);
  EXPECT_EQ(scale_speed<28>(28), 255);
  EXPECT_EQ(scale_speed<126>(126), 255);
}