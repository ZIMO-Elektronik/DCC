#include <array>
#include "bidi_test.hpp"

TEST_F(BiDiTest, app_tos) {
  // Does not require CV28:1
  _cvs[28uz - 1uz] = static_cast<uint8_t>(_cvs[28uz - 1uz] & 0b1111'11101u);
  _addrs.received = _addrs.primary;
  SetUp();

  _addrs.received = {0u, dcc::Address::Broadcast};

  // Make sure to get past backoff (see RCN-218)
  for (auto i{0u}; i < 30.0 / 10E-3; ++i) tipOffSearch();

  // Encode address
  std::array<uint8_t, 2uz> adr;
  encode_address(_addrs.primary, begin(adr));

  // Make datagram
  auto adr_high{
    encode_datagram(make_datagram<Bits::_12>(1u, adr[0uz] & 0b1011'1111u))};
  auto adr_low{encode_datagram(make_datagram<Bits::_12>(2u, adr[1uz]))};
  auto time{encode_datagram(make_datagram<Bits::_12>(14u, 0u))};
  std::array<uint8_t, size(adr_high) + size(adr_low) + size(time)> datagram{};
  auto it{std::copy(cbegin(adr_high), cend(adr_high), begin(datagram))};
  it = std::copy(cbegin(adr_low), cend(adr_low), it);
  std::copy(cbegin(time), cend(time), it);

  EXPECT_CALL(*this, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  cutoutChannel2();
}
