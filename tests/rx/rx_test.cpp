#include "rx_test.hpp"
#include <algorithm>

RxTest::RxTest() {
  _cvs[29uz - 1uz] = 0b1010u; // Decoder configuration
  _cvs[1uz - 1uz] = static_cast<uint8_t>(_addrs.primary); // Primary address
  _cvs[19uz - 1uz] = 0u;           // Consist address low byte
  _cvs[20uz - 1uz] = 0u;           // Consist address high byte
  _cvs[15uz - 1uz] = 0u;           // Lock
  _cvs[16uz - 1uz] = 0u;           // Lock compare
  _cvs[28uz - 1uz] = 0b1000'0011u; // RailCom
  _cvs[250uz - 1uz] = static_cast<uint8_t>(_did >> 24u);  // Decoder ID 1
  _cvs[251uz - 1uz] = static_cast<uint8_t>(_did >> 16u);  // Decoder ID 2
  _cvs[252uz - 1uz] = static_cast<uint8_t>(_did >> 8u);   // Decoder ID 3
  _cvs[253uz - 1uz] = static_cast<uint8_t>(_did >> 0u);   // Decoder ID 4
  _cvs[65297uz - 1uz] = static_cast<uint8_t>(_cid >> 8u); // CID high byte
  _cvs[65298uz - 1uz] = static_cast<uint8_t>(_cid >> 0u); // CID low byte
  _cvs[65299uz - 1uz] = _session_id;                      // Session ID
  _cvs[65300uz - 1uz] = static_cast<uint8_t>(
    0b1100'0000u | _addrs.logon >> 8u); // Logon address high byte
  _cvs[65301uz - 1uz] =
    static_cast<uint8_t>(_addrs.logon >> 0u); // Logon address low byte
}

RxTest::~RxTest() {}

void RxTest::SetUp() {
  EXPECT_CALL(_mock, readCv(_))
    .WillOnce(Return(_cvs[29uz - 1uz]))
    .WillOnce(Return(_cvs[1uz - 1uz]))
    .WillOnce(Return(_cvs[19uz - 1uz]))
    .WillOnce(Return(_cvs[20uz - 1uz]))
    .WillOnce(Return(_cvs[15uz - 1uz]))
    .WillOnce(Return(_cvs[16uz - 1uz]))
    .WillOnce(Return(_cvs[28uz - 1uz]))
    .WillOnce(Return(_cvs[250uz - 1uz]))
    .WillOnce(Return(_cvs[251uz - 1uz]))
    .WillOnce(Return(_cvs[252uz - 1uz]))
    .WillOnce(Return(_cvs[253uz - 1uz]))
    .WillOnce(Return(_cvs[65297uz - 1uz]))
    .WillOnce(Return(_cvs[65298uz - 1uz]))
    .WillOnce(Return(_cvs[65299uz - 1uz]));
  _mock.init();
}

void RxTest::Receive(dcc::Packet const& packet) {
  Receive(dcc::tx::packet2timings(packet));
}

void RxTest::Receive(dcc::tx::Timings const& timings) {
  std::ranges::for_each_n(cbegin(timings),
                          size(timings),
                          [this](uint32_t time) { _mock.receive(time); });
}

void RxTest::Execute() {
  // Receive additional preamble bit before calling execute to avoid being
  // inside a cutout and getting execution blocked!
  _mock.receive(dcc::rx::Timing::Bit1);
  _mock.execute();
}

void RxTest::BiDi() {
  _mock.biDiChannel1();
  _mock.biDiChannel2();
}

void RxTest::EnterServiceMode() {
  EXPECT_CALL(_mock, serviceModeHook(true));
  Receive(dcc::make_reset_packet());
  Execute();
}

void RxTest::ReceiveAndExecuteTwoIdenticalCvWritePackets(uint16_t addr,
                                                         uint32_t cv_addr,
                                                         uint8_t byte) {
  auto packet{dcc::make_cv_access_long_write_packet(addr, cv_addr, byte)};
  for (auto i{0uz}; i < 2uz; ++i) {
    Receive(packet);
    Execute();
  }
}

void RxTest::Logon() {
  EXPECT_CALL(_mock, readCv(_))
    .WillOnce(Return(_cvs[65300uz - 1uz]))
    .WillOnce(Return(_cvs[65301uz - 1uz]));

  // Enable
  Receive(
    dcc::make_logon_enable_packet(dcc::AddressGroup::Now, _cid, _session_id));
}