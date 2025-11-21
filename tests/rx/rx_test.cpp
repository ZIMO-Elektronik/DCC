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

  // Decoder ID
  _cvs[DCC_RX_LOGON_DID_CV_ADDRESS + 0uz] = static_cast<uint8_t>(_did >> 24u);
  _cvs[DCC_RX_LOGON_DID_CV_ADDRESS + 1uz] = static_cast<uint8_t>(_did >> 16u);
  _cvs[DCC_RX_LOGON_DID_CV_ADDRESS + 2uz] = static_cast<uint8_t>(_did >> 8u);
  _cvs[DCC_RX_LOGON_DID_CV_ADDRESS + 3uz] = static_cast<uint8_t>(_did >> 0u);

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
}

RxTest::~RxTest() {}

void RxTest::SetUp() {
  // Extended address
  if (_cvs[29uz - 1uz] & ztl::mask<5u>) {
    /// \note
    /// This is weird... but not having the EXPECT_CALL inside a lambda makes
    /// GCC 14.2.1 (and 15.2.1) hang.
    std::invoke([this] {
      EXTENDED_ADDRESS_EXPECT_CALL_READ_CV_INIT_SEQUENCE();
      _mock.init();
    });
  }
  // Basic address
  else {
    std::invoke([this] {
      BASIC_ADDRESS_EXPECT_CALL_READ_CV_INIT_SEQUENCE();
      _mock.init();
    });
  }
}

RxTest* RxTest::Receive(dcc::Packet const& packet) {
  auto timings{dcc::tx::packet2timings(packet)};
  std::ranges::for_each_n(cbegin(timings),
                          size(timings),
                          [this](uint32_t time) { _mock.receive(time); });
  return this;
}

RxTest* RxTest::BiDiChannel1() {
  _mock.biDiChannel1();
  return this;
}

RxTest* RxTest::BiDiChannel2() {
  _mock.biDiChannel2();
  return this;
}

RxTest* RxTest::BiDi() { return BiDiChannel1()->BiDiChannel2(); }

RxTest* RxTest::LeaveCutout() {
  // Receive additional preamble bit before calling execute to avoid being
  // inside a cutout and getting execution blocked!
  _mock.receive(dcc::rx::Timing::Bit1);
  return this;
}

RxTest* RxTest::Execute() {
  _mock.execute();
  return this;
}

void RxTest::EnterServiceMode() {
  EXPECT_CALL(_mock, serviceModeHook(true));
  Receive(dcc::make_reset_packet())->LeaveCutout()->Execute();
}

void RxTest::Logon() {
  EXPECT_CALL(_mock, readCv(DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 0u))
    .WillRepeatedly(Return(_cvs[DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 0uz]));
  EXPECT_CALL(_mock, readCv(DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 1u))
    .WillRepeatedly(Return(_cvs[DCC_RX_LOGON_ADDRESS_CV_ADDRESS + 1uz]));

  // Enable
  Receive(dcc::make_logon_enable_packet(dcc::LogonGroup::Now, _cid, _sid));
}

dcc::Packet RxTest::TinkerWithPacketLength(dcc::Packet packet) const {
  packet.back() = RandomInterval<uint8_t>(0u, 255u);
  packet.push_back(dcc::exor({cbegin(packet), cend(packet)}));
  return packet;
}
