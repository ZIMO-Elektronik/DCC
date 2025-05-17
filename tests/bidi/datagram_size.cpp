#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <dcc/dcc.hpp>

using namespace dcc::bidi;

TEST(bidi, datagram_size) {
  EXPECT_EQ(datagram_size<Bits::_12>, 2uz);
  EXPECT_EQ(datagram_size<Bits::_18>, 3uz);
  EXPECT_EQ(datagram_size<Bits::_24>, 4uz);
  EXPECT_EQ(datagram_size<Bits::_36>, 6uz);
  EXPECT_EQ(datagram_size<Bits::_48>, 8uz);
}
