#include "rx_test.hpp"
#include <algorithm>

RxTest::RxTest() {
  _cvs[29uz - 1uz] = 0b1010u;   // Decoder configuration
  _cvs[1uz - 1uz] = 3u;         // Primary address
  _cvs[19uz - 1uz] = 0u;        // Consist address low byte
  _cvs[20uz - 1uz] = 0u;        // Consist address high byte
  _cvs[15uz - 1uz] = 0u;        // Lock
  _cvs[16uz - 1uz] = 0u;        // Lock compare
  _cvs[28uz - 1uz] = 0b11u;     // RailCom
  _cvs[250uz - 1uz] = 0xAAu;    // Decoder ID 1
  _cvs[251uz - 1uz] = 0xBBu;    // Decoder ID 2
  _cvs[252uz - 1uz] = 0xCCu;    // Decoder ID 3
  _cvs[253uz - 1uz] = 0xDDu;    // Decoder ID 4
  _cvs[65297uz - 1uz] = 0x00u;  // Logon address low byte
  _cvs[65298uz - 1uz] = 0x2Au;  // Logon address high byte
  _cvs[65299uz - 1uz] = 0xABu;  // CID high byte
  _cvs[65300uz - 1uz] = 0xCDu;  // CID low byte
  _cvs[65301uz - 1uz] = 0x2A;   // Session ID
}

RxTest::~RxTest() {}

void RxTest::SetUp() {
  Expectation read_cv{EXPECT_CALL(_mock, readCv(_))
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
                        .WillOnce(Return(_cvs[65299uz - 1uz]))
                        .WillOnce(Return(_cvs[65300uz - 1uz]))
                        .WillOnce(Return(_cvs[65301uz - 1uz]))};
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

void RxTest::Execute() { _mock.execute(); }

void RxTest::EnterServiceMode() {
  Expectation service_mode_hook{EXPECT_CALL(_mock, serviceModeHook(true))};
  Receive(dcc::make_reset_packet());
  Execute();
}

void RxTest::ReceiveAndExecuteTwoIdenticalCvWritePackets(uint32_t addr,
                                                         uint8_t byte) {
  auto packet{dcc::make_cv_access_long_write_packet(3u, addr, byte)};
  for (auto i{0uz}; i < 2uz; ++i) {
    Receive(packet);
    Execute();
  }
}