#include <gtest/gtest.h>
#include <dcc/dcc.hpp>

using dcc::scale_notch;

TEST(receive_scale_notch, zero) {
  EXPECT_EQ(scale_notch<14>(0), 0);
  EXPECT_EQ(scale_notch<28>(0), 0);
  EXPECT_EQ(scale_notch<126>(0), 0);
}

TEST(receive_scale_notch, one) {
  EXPECT_EQ(scale_notch<14>(1), scale_notch<126>(1));
  EXPECT_EQ(scale_notch<28>(1), scale_notch<126>(1));
}

TEST(receive_scale_notch, two) {
  EXPECT_EQ(scale_notch<14>(2), 21);
  EXPECT_EQ(scale_notch<28>(2), 11);
  EXPECT_EQ(scale_notch<126>(2), 4);
}

TEST(receive_scale_notch, three) {
  EXPECT_EQ(scale_notch<14>(3), 40);
  EXPECT_EQ(scale_notch<28>(3), 20);
  EXPECT_EQ(scale_notch<126>(3), 6);
}

TEST(receive_scale_notch, max) {
  EXPECT_EQ(scale_notch<14>(14), 255);
  EXPECT_EQ(scale_notch<28>(28), 255);
  EXPECT_EQ(scale_notch<126>(126), 255);
}