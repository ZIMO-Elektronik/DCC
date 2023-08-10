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

TEST(address, decode_address) {
  std::array<uint8_t, 2uz> data{};

  data = {};
  EXPECT_EQ(dcc::decode_address(cbegin(data)),
            (dcc::Address{.value = 0u, .type = dcc::Address::Broadcast}));

  data = {0b0000'0011u};
  EXPECT_EQ(dcc::decode_address(cbegin(data)),
            (dcc::Address{.value = 3u, .type = dcc::Address::Short}));

  data = {0b1111'1110u};
  EXPECT_EQ(
    dcc::decode_address(cbegin(data)),
    (dcc::Address{.value = 254u, .type = dcc::Address::AutomaticLogon}));

  data = {0b1111'1111u};
  EXPECT_EQ(dcc::decode_address(cbegin(data)),
            (dcc::Address{.value = 255u, .type = dcc::Address::IdleSystem}));

  data = {0b1101'0011u, 0b0000'1010u};
  EXPECT_EQ(dcc::decode_address(cbegin(data)),
            (dcc::Address{.value = 4874u, .type = dcc::Address::Long}));
}

TEST(address, encode_address) {
  std::array<uint8_t, 2uz> data{};
  std::array<uint8_t, 2uz> expected{};

  expected = {};
  EXPECT_EQ(
    dcc::encode_address(
      dcc::Address{.value = 0u, .type = dcc::Address::Broadcast}, begin(data)),
    cbegin(data) + 1);
  EXPECT_EQ(data, expected);

  expected = {0b0000'0011u};
  EXPECT_EQ(
    dcc::encode_address(dcc::Address{.value = 3u, .type = dcc::Address::Short},
                        begin(data)),
    cbegin(data) + 1);
  EXPECT_EQ(data, expected);

  expected = {0b1111'1110u};
  EXPECT_EQ(dcc::encode_address(
              dcc::Address{.value = 254u, .type = dcc::Address::AutomaticLogon},
              begin(data)),
            cbegin(data) + 1);
  EXPECT_EQ(data, expected);

  expected = {0b1111'1111u};
  EXPECT_EQ(dcc::encode_address(
              dcc::Address{.value = 255u, .type = dcc::Address::IdleSystem},
              begin(data)),
            cbegin(data) + 1);
  EXPECT_EQ(data, expected);

  expected = {0b1101'0011u, 0b0000'1010u};
  EXPECT_EQ(
    dcc::encode_address(
      dcc::Address{.value = 4874u, .type = dcc::Address::Long}, begin(data)),
    cbegin(data) + 2);
  EXPECT_EQ(data, expected);
}