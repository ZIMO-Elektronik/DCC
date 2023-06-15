#include "encode_decode_test.hpp"

#define DATA0 1u, 1u
#define DATA0_NOID 1u << 8u | 1u

// First databyte max 63
#define DATA1 2u, 63u << 8u | 42u
#define DATA1_NOID 2u << 14u | 63u << 8u | 42u

// First databyte max 15
#define DATA2 9u, 15u << 16u | 122u << 8u | 39u
#define DATA2_NOID 9u << 20u | 15u << 16u | 122u << 8u | 39u

#define DATA3 13u, 219u << 24u | 190u << 16u | 74u << 8u | 11u
#define DATA3_NOID                                                             \
  static_cast<uint64_t>(13u) << 32u | 219u << 24u | 190u << 16u | 74u << 8u |  \
    11u

// Encoding and decoding some BiDi messages. All data must be equal
// afterwards.
TEST_F(EncodeDecodeTest, encode_decode) {
  auto e0{encode_datagram(make_datagram<Bits::_12>(DATA0))};
  auto e1{encode_datagram(make_datagram<Bits::_18>(DATA1))};
  auto e2{encode_datagram(make_datagram<Bits::_24>(DATA2))};
  auto e3{encode_datagram(make_datagram<Bits::_36>(DATA3))};

  auto d0{make_data(decode_datagram(e0))};
  auto d1{make_data(decode_datagram(e1))};
  auto d2{make_data(decode_datagram(e2))};
  auto d3{make_data(decode_datagram(e3))};

  EXPECT_EQ(d0, DATA0_NOID);
  EXPECT_EQ(d1, DATA1_NOID);
  EXPECT_EQ(d2, DATA2_NOID);
  EXPECT_EQ(d3, DATA3_NOID);
}

TEST_F(EncodeDecodeTest, too_many_bits) {
  // 2 byte message does not fit 18bit, assertion triggered
  ASSERT_DEBUG_DEATH(make_datagram<Bits::_18>(0u, 64u << 8u), ".*");

  // 3 byte message does not fit 24bit, assertion triggered
  ASSERT_DEBUG_DEATH(make_datagram<Bits::_24>(0u, 16u << 24u), ".*");
}

// make_datagram has 2 overloads, one which takes a seperate ID and one wich
// doesn't. Both should produce the same datagram however.
TEST_F(EncodeDecodeTest, compare_datagram_with_and_without_ids) {
  EXPECT_EQ(make_datagram<Bits::_12>(DATA0),
            make_datagram<Bits::_12>(DATA0_NOID));
  EXPECT_EQ(make_datagram<Bits::_18>(DATA1),
            make_datagram<Bits::_18>(DATA1_NOID));
  EXPECT_EQ(make_datagram<Bits::_24>(DATA2),
            make_datagram<Bits::_24>(DATA2_NOID));
  EXPECT_EQ(make_datagram<Bits::_36>(DATA3),
            make_datagram<Bits::_36>(DATA3_NOID));
}
