#include "rx_test.hpp"

using namespace dcc::bidi;

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
  bytes.emplace_back(0x20u); //
  bytes.resize((size(bytes) + 5uz) / 6uz * 6uz, 0u);
  std::vector<Datagram<datagram_size<Bits::_48>>> get_data{};
  for (auto chunk : bytes | std::views::chunk(6))
    get_data.push_back(encode_datagram(
      make_datagram<Bits::_48>(static_cast<uint64_t>(chunk[0uz]) << 40u |
                               static_cast<uint64_t>(chunk[1uz]) << 32u |
                               static_cast<uint32_t>(chunk[2uz]) << 24u |
                               static_cast<uint32_t>(chunk[3uz]) << 16u |
                               static_cast<uint32_t>(chunk[4uz]) << 8u |
                               static_cast<uint32_t>(chunk[5uz]) << 0u)));

  // Transmit it within 6 cutouts
  EXPECT_CALL(
    _mock,
    transmitBiDi(DatagramMatcher(std::span{cbegin(get_data[0uz]), 2uz})));
  EXPECT_CALL(
    _mock,
    transmitBiDi(DatagramMatcher(std::span{cbegin(get_data[0uz]) + 2, 6uz})));
  Receive(dcc::make_get_data_start_packet())->BiDi();

  EXPECT_CALL(
    _mock,
    transmitBiDi(DatagramMatcher(std::span{cbegin(get_data[1uz]), 2uz})));
  EXPECT_CALL(
    _mock,
    transmitBiDi(DatagramMatcher(std::span{cbegin(get_data[1uz]) + 2, 6uz})));
  Receive(dcc::make_get_data_cont_packet())->BiDi();

  EXPECT_CALL(
    _mock,
    transmitBiDi(DatagramMatcher(std::span{cbegin(get_data[2uz]), 2uz})));
  EXPECT_CALL(
    _mock,
    transmitBiDi(DatagramMatcher(std::span{cbegin(get_data[2uz]) + 2, 6uz})));
  Receive(dcc::make_get_data_cont_packet())->BiDi();

  EXPECT_CALL(
    _mock,
    transmitBiDi(DatagramMatcher(std::span{cbegin(get_data[3uz]), 2uz})));
  EXPECT_CALL(
    _mock,
    transmitBiDi(DatagramMatcher(std::span{cbegin(get_data[3uz]) + 2, 6uz})));
  Receive(dcc::make_get_data_cont_packet())->BiDi();

  EXPECT_CALL(
    _mock,
    transmitBiDi(DatagramMatcher(std::span{cbegin(get_data[4uz]), 2uz})));
  EXPECT_CALL(
    _mock,
    transmitBiDi(DatagramMatcher(std::span{cbegin(get_data[4uz]) + 2, 6uz})));
  Receive(dcc::make_get_data_cont_packet())->BiDi();

  EXPECT_CALL(
    _mock,
    transmitBiDi(DatagramMatcher(std::span{cbegin(get_data[5uz]), 2uz})));
  EXPECT_CALL(
    _mock,
    transmitBiDi(DatagramMatcher(std::span{cbegin(get_data[5uz]) + 2, 6uz})));
  Receive(dcc::make_get_data_cont_packet())->BiDi();
}
