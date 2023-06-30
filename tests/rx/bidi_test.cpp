#include "bidi_test.hpp"

BiDiTest::BiDiTest() {
  _cvs[29uz - 1uz] = 0b1000u;   // Decoder configuration
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

  Expectation read_cv{EXPECT_CALL(*this, readCv(_))
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
  init();
}

BiDiTest::~BiDiTest() {}