#include <array>
#include "bidi_test.hpp"

TEST_F(BiDiTest, app_tos) {
  for (auto i{0u}; i < 30.0 / 10E-3; ++i)
    tipOffSearch();
  _addrs.received = {0u, dcc::Address::Broadcast};

  // Make datagram
  auto adr_high{encode_datagram(make_datagram<Bits::_12>(1u, 0u))};
  auto adr_low{
    encode_datagram(make_datagram<Bits::_12>(2u, _addrs.primary & 0xFFu))};
  auto time{encode_datagram(make_datagram<Bits::_12>(14u, 0u))};
  std::array<uint8_t, size(adr_high) + size(adr_low) + size(time)> datagram{};
  auto it{std::copy(cbegin(adr_high), cend(adr_high), begin(datagram))};
  it = std::copy(cbegin(adr_low), cend(adr_low), it);
  std::copy(cbegin(time), cend(time), it);

  EXPECT_CALL(*this, transmitBiDi(DatagramMatcher(datagram))).Times(AtLeast(1));
  cutoutChannel2();
}