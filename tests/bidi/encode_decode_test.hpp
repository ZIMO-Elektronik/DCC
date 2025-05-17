#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <dcc/dcc.hpp>

using namespace dcc::bidi;

// BiDi encode-decode test fixture
struct EncodeDecodeTest : ::testing::Test {
protected:
  EncodeDecodeTest();
  virtual ~EncodeDecodeTest();
};

MATCHER_P(DatagramMatcher, datagram, "") {
  return std::ranges::equal(datagram, arg);
}
