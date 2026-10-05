#include "rx_test.hpp"

using namespace dcc::bidi;

namespace {

// Make vector of get_data datagrams from CVs and data space index
std::vector<Datagram<datagram_size<Bits::_48>>>
make_get_data_datagrams(std::span<uint8_t const> cvs, uint8_t data_space) {
  std::vector<uint8_t> bytes;
  uint8_t crc;

  // Bytes
  for (auto i{0uz}; i < size(cvs); ++i) {
    // Header: 0, 33, 66, 99, ...
    if (!(i % 31uz)) {
      auto const block_size{std::min<size_t>(31uz, size(cvs) - i)};
      bytes.emplace_back(
        static_cast<uint8_t>((i ? ztl::mask<5u> : 0u) | block_size));
      crc = dcc::crc8(bytes.back() ^ data_space);
    }

    // Data
    bytes.emplace_back(cvs[i]);
    crc = dcc::crc8(bytes.back() ^ crc);

    // CRC: 32, 65, 98, 131, ...
    if (i % 31uz == std::min<size_t>(31uz, size(cvs) - (i / 31uz) * 31uz) - 1uz)
      bytes.emplace_back(crc);
  }

  // Special continuation block (header only)
  if (!empty(cvs) && !(size(cvs) % 31uz)) bytes.emplace_back(ztl::mask<5u>);

  // Resize to multiple of 6 (cutout size)
  bytes.resize((size(bytes) + 5uz) / 6uz * 6uz, 0u);

  std::vector<Datagram<datagram_size<Bits::_48>>> retval;
  for (auto chunk : bytes | std::views::chunk(6))
    retval.push_back(make_get_data_datagram(
      std::span<uint8_t, 6uz>{cbegin(chunk), cend(chunk)}));

  return retval;
}

} // namespace

TEST_F(RxTest, read_data_space_0) {
  Logon();

  // Confirm LOGON_SELECT via 8x ACK
  InSequence s;
  EXPECT_CALL(_mock,
              transmitBiDi(DatagramMatcher(std::array{acks[0uz], acks[0uz]})));
  EXPECT_CALL(
    _mock,
    transmitBiDi(DatagramMatcher(std::array{
      acks[0uz], acks[0uz], acks[0uz], acks[0uz], acks[0uz], acks[0uz]})));
  Receive(
    dcc::make_logon_select_packet(DCC_MANUFACTURER_ID, _did, 0b1111'1110u, 0u));
  BiDi();

  // Read 31 CVs from data space 0
  std::span cvs{&_cvs[2uz * smath::pow(256uz, 2uz) + 0uz * 256uz], 31uz};
  auto i{0uz};
  EXPECT_CALL(_mock, readCv(_)).Times(31).WillRepeatedly([&] {
    return cvs[i++];
  });
  LeaveCutout()->Execute();

  // Make get_data datagrams
  auto get_data{make_get_data_datagrams(cvs, 0u)};

  for (i = 0uz; i < size(get_data); ++i) {
    EXPECT_CALL(
      _mock,
      transmitBiDi(DatagramMatcher(std::span{cbegin(get_data[i]), 2uz})));
    EXPECT_CALL(
      _mock,
      transmitBiDi(DatagramMatcher(std::span{cbegin(get_data[i]) + 2, 6uz})));
    auto packet{!i ? dcc::make_get_data_start_packet()
                   : dcc::make_get_data_cont_packet()};
    Receive(packet)->BiDi();
  }
}

TEST_F(RxTest, read_data_space_1) {
  Logon();

  // Confirm LOGON_SELECT via 8x ACK
  InSequence s;
  EXPECT_CALL(_mock,
              transmitBiDi(DatagramMatcher(std::array{acks[0uz], acks[0uz]})));
  EXPECT_CALL(
    _mock,
    transmitBiDi(DatagramMatcher(std::array{
      acks[0uz], acks[0uz], acks[0uz], acks[0uz], acks[0uz], acks[0uz]})));
  Receive(
    dcc::make_logon_select_packet(DCC_MANUFACTURER_ID, _did, 0b1111'1110u, 1u));
  BiDi();

  // Read 32 CVs from data space 1
  std::span cvs{&_cvs[2uz * smath::pow(256uz, 2uz) + 1uz * 256uz], 32uz};
  auto i{0uz};
  EXPECT_CALL(_mock, readCv(_)).Times(32).WillRepeatedly([&] {
    return cvs[i++];
  });
  LeaveCutout()->Execute();

  // Make get_data datagrams
  auto get_data{make_get_data_datagrams(cvs, 1u)};

  for (i = 0uz; i < size(get_data); ++i) {
    EXPECT_CALL(
      _mock,
      transmitBiDi(DatagramMatcher(std::span{cbegin(get_data[i]), 2uz})));
    EXPECT_CALL(
      _mock,
      transmitBiDi(DatagramMatcher(std::span{cbegin(get_data[i]) + 2, 6uz})));
    auto packet{!i ? dcc::make_get_data_start_packet()
                   : dcc::make_get_data_cont_packet()};
    Receive(packet)->BiDi();
  }
}
