#include <array>
#include "rx_test.hpp"

using namespace dcc::bidi;
using namespace std::chrono_literals;

TEST_F(RxTest, app_tos_basic_address) {
  // Does not require CV28:1
  _cvs[28uz - 1uz] = static_cast<uint8_t>(_cvs[28uz - 1uz] & 0b1111'11101u);
  SetUp();

  // Make sure to get past backoff (see RCN-218)
  for (auto i{0.0}; i < 30.0 / 10E-3; ++i)
    LeaveCutout()->Execute()->Receive(
      dcc::make_binary_state_short_packet(0u, 2u));

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
  _mock.biDiChannel2();
}

TEST_F(RxTest, app_tos_extended_address) {
  _addrs.primary = {.value = 3000u, .type = dcc::Address::ExtendedLoco};
  dcc::encode_address(_addrs.primary, begin(_cvs) + 17 - 1);
  _cvs[29uz - 1uz] = _cvs[29uz - 1uz] | ztl::mask<5u>;
  SetUp();

  // Make sure to get past backoff (see RCN-218)
  for (auto i{0.0}; i < 30.0 / 10E-3; ++i)
    LeaveCutout()->Execute()->Receive(
      dcc::make_binary_state_short_packet(0u, 2u));

  // Make datagram
  auto adr_high{encode_datagram(
    make_datagram<Bits::_12>(1u, 0x80u | (_addrs.primary & 0x3F00u) >> 8u))};
  auto adr_low{encode_datagram(
    make_datagram<Bits::_12>(2u, static_cast<uint8_t>(_addrs.primary)))};
  auto time{encode_datagram(make_datagram<Bits::_12>(14u, 0u))};
  std::array<uint8_t, size(adr_high) + size(adr_low) + size(time)> datagram{};
  auto it{std::copy(cbegin(adr_high), cend(adr_high), begin(datagram))};
  it = std::copy(cbegin(adr_low), cend(adr_low), it);
  std::copy(cbegin(time), cend(time), it);

  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  _mock.biDiChannel2();
}

TEST_F(
  RxTest,
  app_tos_time_is_from_first_packet_regardless_of_which_packet_is_answered) {
  SetUp();

  // Make sure to get past backoff (see RCN-218)
  for (auto i{0.0}; i < 30.0 / 10E-3; ++i)
    LeaveCutout()->Execute()->Receive(
      dcc::make_binary_state_short_packet(0u, 2u));

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
  _mock.biDiChannel2();

  std::this_thread::sleep_for(1s);

  // Make sure to get past backoff (see RCN-218)
  for (auto i{0.0}; i < 30.0 / 10E-3; ++i)
    LeaveCutout()->Execute()->Receive(
      dcc::make_binary_state_short_packet(0u, 2u));

  EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagram))).Times(1);
  _mock.biDiChannel2();
}
