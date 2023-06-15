#include <gtest/gtest.h>
#include <dcc/dcc.hpp>

TEST(address, short_and_long_address_compare_equal_based_solely_on_value) {
  dcc::Address short_addr{.value = 3u, .type = dcc::Address::Short};
  dcc::Address long_addr{.value = 3u, .type = dcc::Address::Long};
  EXPECT_EQ(short_addr, long_addr);
  EXPECT_EQ(short_addr, 3u);
  EXPECT_EQ(long_addr, 3u);
}

TEST(address, short_and_accessory_address_are_not_equal_despite_same_value) {
  dcc::Address short_addr{.value = 3u, .type = dcc::Address::Short};
  dcc::Address accessory_addr{.value = 3u, .type = dcc::Address::Accessory};
  EXPECT_NE(short_addr, accessory_addr);
  EXPECT_EQ(short_addr, 3u);
  EXPECT_EQ(accessory_addr, 3u);
}