#include <gtest/gtest.h>
#include <dcc/dcc.hpp>

TEST(address,
     basic_and_extended_loco_address_compare_equal_based_solely_on_value) {
  dcc::Address short_addr{.value = 3u, .type = dcc::Address::BasicLoco};
  dcc::Address long_addr{.value = 3u, .type = dcc::Address::ExtendedLoco};
  EXPECT_EQ(short_addr, long_addr);
  EXPECT_EQ(short_addr, 3u);
  EXPECT_EQ(long_addr, 3u);
}

TEST(address,
     basic_loco_and_basic_accessory_address_are_not_equal_despite_same_value) {
  dcc::Address short_addr{.value = 3u, .type = dcc::Address::BasicLoco};
  dcc::Address basic_accessory_addr{.value = 3u,
                                    .type = dcc::Address::BasicAccessory};
  EXPECT_NE(short_addr, basic_accessory_addr);
  EXPECT_EQ(short_addr, 3u);
  EXPECT_EQ(basic_accessory_addr, 3u);
}

TEST(address,
     basic_and_extended_accessory_address_are_not_equal_despite_same_value) {
  dcc::Address basic_accessory_addr{.value = 3u,
                                    .type = dcc::Address::BasicAccessory};
  dcc::Address extended_accessory_addr{.value = 3u,
                                       .type = dcc::Address::ExtendedAccessory};
  EXPECT_NE(basic_accessory_addr, extended_accessory_addr);
  EXPECT_EQ(basic_accessory_addr, 3u);
  EXPECT_EQ(extended_accessory_addr, 3u);
}

TEST(address, decode_address) {
  {
    std::array<uint8_t, 2uz> data{};
    EXPECT_EQ(dcc::decode_address(cbegin(data)),
              (dcc::Address{.value = 0u, .type = dcc::Address::Broadcast}));
  }

  {
    std::array<uint8_t, 2uz> data{0b0000'0011u};
    EXPECT_EQ(dcc::decode_address(cbegin(data)),
              (dcc::Address{.value = 3u, .type = dcc::Address::BasicLoco}));
  }

  {
    std::array<uint8_t, 2uz> data{0b1011'0011u, 0b1010'0010u};
    EXPECT_EQ(
      dcc::decode_address(cbegin(data)),
      (dcc::Address{.value = 717u, .type = dcc::Address::BasicAccessory}));
  }

  {
    std::array<uint8_t, 2uz> data{0b1011'0011u, 0b0010'0011u};
    EXPECT_EQ(
      dcc::decode_address(cbegin(data)),
      (dcc::Address{.value = 717u, .type = dcc::Address::ExtendedAccessory}));
  }

  {
    std::array<uint8_t, 2uz> data{0b1101'0011u, 0b0000'1010u};
    EXPECT_EQ(
      dcc::decode_address(cbegin(data)),
      (dcc::Address{.value = 4874u, .type = dcc::Address::ExtendedLoco}));
  }

  {
    std::array<uint8_t, 2uz> data{0b1111'1110u};
    EXPECT_EQ(
      dcc::decode_address(cbegin(data)),
      (dcc::Address{.value = 254u, .type = dcc::Address::AutomaticLogon}));
  }

  {
    std::array<uint8_t, 2uz> data{0b1111'1111u};
    EXPECT_EQ(dcc::decode_address(cbegin(data)),
              (dcc::Address{.value = 255u, .type = dcc::Address::IdleSystem}));
  }
}

TEST(address, encode_address) {
  {
    std::array<uint8_t, 2uz> data{};
    EXPECT_EQ(dcc::encode_address(
                dcc::Address{.value = 0u, .type = dcc::Address::Broadcast},
                begin(data)),
              cbegin(data) + 1);
    EXPECT_EQ(data, (decltype(data){}));
  }

  {
    std::array<uint8_t, 2uz> data{};
    EXPECT_EQ(dcc::encode_address(
                dcc::Address{.value = 3u, .type = dcc::Address::BasicLoco},
                begin(data)),
              cbegin(data) + 1);
    EXPECT_EQ(data, (decltype(data){0b0000'0011u}));
  }

  {
    std::array<uint8_t, 2uz> data{};
    EXPECT_EQ(
      dcc::encode_address(
        dcc::Address{.value = 717u, .type = dcc::Address::BasicAccessory},
        begin(data)),
      cbegin(data) + 2);
    EXPECT_EQ(data, (decltype(data){0b1011'0011u, 0b1010'0010u}));
  }

  {
    std::array<uint8_t, 2uz> data{};
    EXPECT_EQ(
      dcc::encode_address(
        dcc::Address{.value = 717u, .type = dcc::Address::ExtendedAccessory},
        begin(data)),
      cbegin(data) + 2);
    EXPECT_EQ(data, (decltype(data){0b1011'0011u, 0b0010'0011u}));
  }

  {
    std::array<uint8_t, 2uz> data{};
    EXPECT_EQ(
      dcc::encode_address(
        dcc::Address{.value = 4874u, .type = dcc::Address::ExtendedLoco},
        begin(data)),
      cbegin(data) + 2);
    EXPECT_EQ(data, (decltype(data){0b1101'0011u, 0b0000'1010u}));
  }

  {
    std::array<uint8_t, 2uz> data{};
    EXPECT_EQ(
      dcc::encode_address(
        dcc::Address{.value = 254u, .type = dcc::Address::AutomaticLogon},
        begin(data)),
      cbegin(data) + 1);
    EXPECT_EQ(data, (decltype(data){0b1111'1110u}));
  }

  {
    std::array<uint8_t, 2uz> data{};
    EXPECT_EQ(dcc::encode_address(
                dcc::Address{.value = 255u, .type = dcc::Address::IdleSystem},
                begin(data)),
              cbegin(data) + 1);
    EXPECT_EQ(data, (decltype(data){0b1111'1111u}));
  }
}