#include "rx_test.hpp"

TEST_F(RxTest, read_data_space_0) {
  Logon();

  // Confirm via 8x ACK
  InSequence s;
  EXPECT_CALL(_mock,
              transmitBiDi(DatagramMatcher(
                std::array{dcc::bidi::acks[0uz], dcc::bidi::acks[0uz]})));
  EXPECT_CALL(_mock,
              transmitBiDi(DatagramMatcher(std::array{dcc::bidi::acks[0uz],
                                                      dcc::bidi::acks[0uz],
                                                      dcc::bidi::acks[0uz],
                                                      dcc::bidi::acks[0uz],
                                                      dcc::bidi::acks[0uz],
                                                      dcc::bidi::acks[0uz]})));
  Receive(
    dcc::make_logon_select_packet(DCC_MANUFACTURER_ID, _did, 0b1111'1110u, 0u));
  BiDi();

  // Read data space 0
  EXPECT_CALL(_mock, readCv(_))
    .Times(31)
    .WillRepeatedly([count = 0u](uint32_t) mutable { return count++; });
  LeaveCutout()->Execute();

  // Transmit it within 6 cutouts
  std::vector<uint8_t> bytes;
  bytes.emplace_back(31u);
  for (uint8_t i{0u}; i < 31u; ++i) bytes.emplace_back(i);
  bytes.emplace_back(dcc::crc8(bytes));
  bytes.resize(36uz, 0u);
  std::array<
    dcc::bidi::Datagram<dcc::bidi::datagram_size<dcc::bidi::Bits::_48>>,
    6uz>
    datagrams{};
  for (auto [i, chunk] :
       std::views::zip(std::views::iota(0uz), bytes | std::views::chunk(6)))
    datagrams[i] =
      dcc::bidi::encode_datagram(dcc::bidi::make_datagram<dcc::bidi::Bits::_48>(
        static_cast<uint64_t>(chunk[0uz]) << 40u |
        static_cast<uint64_t>(chunk[1uz]) << 32u |
        static_cast<uint32_t>(chunk[2uz]) << 24u |
        static_cast<uint32_t>(chunk[3uz]) << 16u |
        static_cast<uint32_t>(chunk[4uz]) << 8u |
        static_cast<uint32_t>(chunk[5uz]) << 0u));
  // EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagrams[0uz])));
  // EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagrams[1uz])));
  // EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagrams[2uz])));
  // EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagrams[3uz])));
  // EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagrams[4uz])));
  // EXPECT_CALL(_mock, transmitBiDi(DatagramMatcher(datagrams[5uz])));
}
