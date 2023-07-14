#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <dcc/dcc.hpp>

#pragma once

using namespace dcc::bidi;

// BiDi encode-decode test fixture
struct EncodeDecodeTest : ::testing::Test {
  EncodeDecodeTest();
  virtual ~EncodeDecodeTest();
};

MATCHER_P(DatagramMatcher, datagram, "") {
  return std::equal(cbegin(datagram), cend(datagram), arg);
}