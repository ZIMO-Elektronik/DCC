#include <array>
#include "rx_test.hpp"

using namespace dcc::bidi;

TEST_F(RxTest, app_tos) {
  // Does not require CV28:1
  _cvs[28uz - 1uz] = static_cast<uint8_t>(_cvs[28uz - 1uz] & 0b1111'11101u);
  SetUp();

  // Make sure to get past backoff (see RCN-218)
  for (auto i{0u}; i < 30.0 / 10E-3; ++i) {
    Receive(dcc::make_binary_state_short_packet(0u, 2u));
    Execute();
  }

  // Make datagram
  auto adr_high{encode_datagram(make_datagram<Bits::_12>(1u, 0u))};
  auto adr_low{encode_datagram(
    make_datagram<Bits::_12>(2u, static_cast<uint8_t>(_addrs.primary)))};
  auto time{encode_datagram(make_datagram<Bits::_12>(14u, 0u))};
  std::array<uint8_t, size(adr_high) + size(adr_low) + size(time)> datagram{};
  auto it{std::copy(cbegin(adr_high), cend(adr_high), begin(datagram))};
  it = std::copy(cbegin(adr_low), cend(adr_low), it);
  std::copy(cbegin(time), cend(time), it);

  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  _mock.cutoutChannel2();
}
