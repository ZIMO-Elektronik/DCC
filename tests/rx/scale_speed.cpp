#include <gtest/gtest.h>
#include <dcc/dcc.hpp>

TEST(receive_scale_speed, estop) {
  EXPECT_EQ(dcc::scale_speed<14>(dcc::EStop), dcc::EStop);
  EXPECT_EQ(dcc::scale_speed<28>(dcc::EStop), dcc::EStop);
  EXPECT_EQ(dcc::scale_speed<126>(dcc::EStop), dcc::EStop);
}

TEST(receive_scale_speed, stop) {
  EXPECT_EQ(dcc::scale_speed<14>(dcc::Stop), dcc::Stop);
  EXPECT_EQ(dcc::scale_speed<28>(dcc::Stop), dcc::Stop);
  EXPECT_EQ(dcc::scale_speed<126>(dcc::Stop), dcc::Stop);
}

TEST(receive_scale_speed, one) {
  EXPECT_EQ(dcc::scale_speed<14>(1), dcc::scale_speed<126>(1));
  EXPECT_EQ(dcc::scale_speed<28>(1), dcc::scale_speed<126>(1));
}

TEST(receive_scale_speed, two) {
  EXPECT_EQ(dcc::scale_speed<14>(2), 21);
  EXPECT_EQ(dcc::scale_speed<28>(2), 11);
  EXPECT_EQ(dcc::scale_speed<126>(2), 4);
}

TEST(receive_scale_speed, three) {
  EXPECT_EQ(dcc::scale_speed<14>(3), 40);
  EXPECT_EQ(dcc::scale_speed<28>(3), 20);
  EXPECT_EQ(dcc::scale_speed<126>(3), 6);
}

TEST(receive_scale_speed, max) {
  EXPECT_EQ(dcc::scale_speed<14>(14), 255);
  EXPECT_EQ(dcc::scale_speed<28>(28), 255);
  EXPECT_EQ(dcc::scale_speed<126>(126), 255);
}
