#pragma once

#include "../rx_mock.hpp"

using namespace dcc::bidi;
using namespace ::testing;

// BiDi test fixture
struct BiDiTest : ::testing::Test, RxMock {
protected:
  BiDiTest();
  virtual ~BiDiTest();

  void SetUp() override;

  std::array<uint8_t, smath::pow(2uz, 16uz)> _cvs{};
};

MATCHER_P(DatagramMatcher, datagram, "") {
  return std::equal(cbegin(datagram), cend(datagram), cbegin(arg));
}