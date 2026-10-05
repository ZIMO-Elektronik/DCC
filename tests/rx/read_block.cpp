#include "rx_test.hpp"

using namespace dcc::bidi;

namespace {

std::vector<Datagram<datagram_size<Bits::_48>>>
make_get_data_datagrams(std::span<uint8_t const> bytes) {
  std::vector<Datagram<datagram_size<Bits::_48>>> retval;

  // TODO
  // 1.) Alle 31 Byte Header / CRC
  // 2.)
  // 3.) append 0 bis vielfaches von 6

  return retval;
}

} // namespace

TEST_F(RxTest, read_data_space_0) {
  Logon();

  // Confirm via 8x ACK
  InSequence s;
  std::array ack{acks[0uz],
                 acks[0uz],
                 acks[0uz],
                 acks[0uz],
                 acks[0uz],
                 acks[0uz],
                 acks[0uz],
                 acks[0uz]};
  EXPECT_CALL(_mock,
              transmitBiDi(DatagramMatcher(std::span{cbegin(ack), 2uz})));
  EXPECT_CALL(_mock,
              transmitBiDi(DatagramMatcher(std::span{cbegin(ack) + 2, 6uz})));
  Receive(
    dcc::make_logon_select_packet(DCC_MANUFACTURER_ID, _did, 0b1111'1110u, 0u));
  BiDi();

  // Read 31 CVs from data space 0
  auto const cv_addr{2uz * smath::pow(256uz, 2uz) + 0uz * 256uz};
  auto i{cv_addr};
  EXPECT_CALL(_mock, readCv(_)).Times(31).WillRepeatedly([&] {
    return _cvs[i++];
  });
  LeaveCutout()->Execute();

  // Transform raw bytes to datagrams
  std::vector<uint8_t> bytes;
  bytes.emplace_back(31u);
  std::copy_n(&_cvs[cv_addr], 31uz, std::back_inserter(bytes));
  bytes.emplace_back(dcc::crc8(bytes));
  bytes.emplace_back(0x20u); // Special continuation block (header only)
  bytes.resize((size(bytes) + 5uz) / 6uz * 6uz, 0u);
  std::vector<Datagram<datagram_size<Bits::_48>>> get_data{};
  for (auto chunk : bytes | std::views::chunk(6))
    get_data.push_back(make_get_data_datagram(
      std::span<uint8_t, 6uz>{cbegin(chunk), cend(chunk)}));

  // Transmit it within 6 cutouts
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
