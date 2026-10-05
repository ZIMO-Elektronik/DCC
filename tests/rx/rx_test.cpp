#include "rx_test.hpp"
#include <algorithm>
#include <cassert>
#include "../utility.hpp"

RxTest::RxTest() {
  _last_packet = {}; // Clear static packet

  _cvs[29uz - 1uz] = 0b1010u; // Decoder configuration
  _cvs[1uz - 1uz] = static_cast<uint8_t>(_addrs.primary); // Primary address
  _cvs[19uz - 1uz] = 0u;           // Consist address low byte
  _cvs[20uz - 1uz] = 0u;           // Consist address high byte
  _cvs[15uz - 1uz] = 0u;           // Lock
  _cvs[16uz - 1uz] = 0u;           // Lock compare
  _cvs[28uz - 1uz] = 0b1000'0011u; // RailCom

  // Decoder ID (little endian)
  _cvs[DCC_RX_LOGON_DID_CV_ADDRESS + 0uz] = static_cast<uint8_t>(_did >> 0u);
  _cvs[DCC_RX_LOGON_DID_CV_ADDRESS + 1uz] = static_cast<uint8_t>(_did >> 8u);
  _cvs[DCC_RX_LOGON_DID_CV_ADDRESS + 2uz] = static_cast<uint8_t>(_did >> 16u);
  _cvs[DCC_RX_LOGON_DID_CV_ADDRESS + 3uz] = static_cast<uint8_t>(_did >> 24u);

  // CID
  _cvs[DCC_RX_LOGON_CID_CV_ADDRESS + 0uz] = static_cast<uint8_t>(_cid >> 8u);
  _cvs[DCC_RX_LOGON_CID_CV_ADDRESS + 1uz] = static_cast<uint8_t>(_cid >> 0u);

  // SID
  _cvs[DCC_RX_LOGON_SID_CV_ADDRESS] = _sid;

  // Logon address
  _cvs[DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 0uz] =
    static_cast<uint8_t>(0b1100'0000u | _addrs.logon >> 8u);
  _cvs[DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 1uz] =
    static_cast<uint8_t>(_addrs.logon >> 0u);

  // iota data spaces beginning with their data space number
  for (uint8_t i{0u}; i < 8u; ++i) {
    auto const cv_addr{2uz * smath::pow(256uz, 2uz) + i * 256uz};
    std::iota(&_cvs[cv_addr], &_cvs[cv_addr + 256uz], i);
  }
}

RxTest::~RxTest() {}

void RxTest::SetUp() {
  // Extended address
  if (_cvs[29uz - 1uz] & ztl::mask<5u>) {
    /// \note
    /// This is weird... but not having the EXPECT_CALL inside a lambda makes
    /// GCC 14.2.1 (and 15.2.1) hang.
    std::invoke([this] {
      EXPECT_CALL(_mock, readCv(_)).EXTENDED_ADDRESS_READ_CV_INIT_SEQUENCE();
      _mock.init();
    });
  }
  // Basic address
  else {
    std::invoke([this] {
      EXPECT_CALL(_mock, readCv(_)).BASIC_ADDRESS_READ_CV_INIT_SEQUENCE();
      _mock.init();
    });
  }
}

RxTest* RxTest::Receive(dcc::Packet const& packet, dcc::tx::Config cfg) {
  _last_packet = packet;
  auto timings{dcc::tx::packet2timings(packet, cfg)};
  std::ranges::for_each_n(cbegin(timings),
                          size(timings),
                          [this](uint32_t time) { _mock.receive(time); });
  return this;
}

// Receive any packet to enter cutout
RxTest* RxTest::EnterCutout(dcc::Packet const& packet) {
  EXPECT_NE(packet, dcc::Packet{});
  return Receive(packet);
}

RxTest* RxTest::BiDiChannel1() {
  _mock.biDiChannel1();
  return this;
}

RxTest* RxTest::BiDiChannel2() {
  _mock.biDiChannel2();
  return this;
}

// Receive additional preamble bit before calling execute to avoid being inside
// a cutout and getting execution blocked!
RxTest* RxTest::LeaveCutout() {
  _mock.receive(dcc::rx::Timing::Bit1);
  return this;
}

RxTest* RxTest::Execute() {
  _mock.execute();
  return this;
}

RxTest* RxTest::ReceiveAndExecute(dcc::Packet const& packet,
                                  dcc::tx::Config cfg) {
  return Receive(packet, cfg)->LeaveCutout()->Execute();
}

RxTest* RxTest::ReceiveAndExecuteTwice(dcc::Packet const& packet,
                                       dcc::tx::Config cfg) {
  return ReceiveAndExecute(packet, cfg)->ReceiveAndExecute(packet, cfg);
}

RxTest* RxTest::BiDi() { return BiDiChannel1()->BiDiChannel2(); }

void RxTest::EnterServiceMode() {
  EXPECT_CALL(_mock, serviceModeHook(true));
  ReceiveAndExecute(dcc::make_reset_packet());
}

// Quick logon with known CID and SID
void RxTest::Logon() {
  EXPECT_CALL(_mock, readCv(DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 0u))
    .WillRepeatedly(Return(_cvs[DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 0uz]));
  EXPECT_CALL(_mock, readCv(DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 1u))
    .WillRepeatedly(Return(_cvs[DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 1uz]));
  // Store assignment
  EXPECT_CALL(_mock, writeCv(_, _)).Times(AtLeast(7));
  ReceiveAndExecute(
    dcc::make_logon_enable_packet(dcc::LogonGroup::Now, _cid, _sid));
}

// Read data space
void RxTest::ReadDataSpace(uint8_t data_space,
                           uint32_t cv_addr,
                           uint8_t cv_count) {
  static constexpr std::array data_space_sizes{
    31uz,                      // Extended capabilities
    32uz,                      // SpaceInfo
    28uz,                      // ShortGUI
    31uz,                      // CV-Read
    256uz,                     // Icons
    63uz + 63uz,               // Long name
    41uz + 41uz + 21uz + 21uz, // Product information
    16uz + 16uz + 92uz};       // Vehicle-specific information
  assert(cv_count <= data_space_sizes[3uz]);

  Logon();

  // Confirm LOGON_SELECT via 8x ACK
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
  auto packet{data_space == 3u
                ? dcc::make_logon_select_packet(DCC_MANUFACTURER_ID,
                                                _did,
                                                0b1111'1110u,
                                                data_space,
                                                cv_addr,
                                                cv_count)
                : dcc::make_logon_select_packet(
                    DCC_MANUFACTURER_ID, _did, 0b1111'1110u, data_space)};
  Receive(packet)->BiDi();

  // Read CVs from data space
  auto data_space_size{data_space == 3u ? cv_count
                                        : data_space_sizes[data_space]};
  std::span cvs{
    &_cvs[data_space == 3u ? cv_addr
                           : 2uz * smath::pow(256uz, 2uz) + data_space * 256uz],
    data_space_size};
  auto i{0uz};
  EXPECT_CALL(_mock, readCv(_))
    .Times(static_cast<int>(data_space_size))
    .WillRepeatedly([&] { return cvs[i++]; });
  LeaveCutout()->Execute();

  // Make get_data datagrams and query them
  auto get_data{make_get_data_datagrams(cvs, data_space)};
  for (i = 0uz; i < size(get_data); ++i) {
    EXPECT_CALL(
      _mock,
      transmitBiDi(DatagramMatcher(std::span{cbegin(get_data[i]), 2uz})));
    EXPECT_CALL(
      _mock,
      transmitBiDi(DatagramMatcher(std::span{cbegin(get_data[i]) + 2, 6uz})));
    packet =
      !i ? dcc::make_get_data_start_packet() : dcc::make_get_data_cont_packet();
    Receive(packet)->BiDi();
  }
}

// Tinker with the length of a valid packet
dcc::Packet RxTest::TinkerWithPacketLength(dcc::Packet packet) const {
  packet.back() = random_interval<uint8_t>(0u, 255u);
  packet.push_back(dcc::exor({cbegin(packet), cend(packet)}));
  return packet;
}
