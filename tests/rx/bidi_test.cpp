#include "bidi_test.hpp"

BiDiTest::BiDiTest() {
  cvs_[29uz - 1uz] = 0b1000u;   // Decoder configuration
  cvs_[1uz - 1uz] = 3u;         // Primary address
  cvs_[19uz - 1uz] = 0u;        // Consist address low byte
  cvs_[20uz - 1uz] = 0u;        // Consist address high byte
  cvs_[15uz - 1uz] = 0u;        // Lock
  cvs_[16uz - 1uz] = 0u;        // Lock compare
  cvs_[28uz - 1uz] = 0b11u;     // RailCom
  cvs_[250uz - 1uz] = 0xAAu;    // Decoder ID 1
  cvs_[251uz - 1uz] = 0xBBu;    // Decoder ID 2
  cvs_[252uz - 1uz] = 0xCCu;    // Decoder ID 3
  cvs_[253uz - 1uz] = 0xDDu;    // Decoder ID 4
  cvs_[65297uz - 1uz] = 0x00u;  // Logon address low byte
  cvs_[65298uz - 1uz] = 0x2Au;  // Logon address high byte
  cvs_[65299uz - 1uz] = 0xABu;  // CID high byte
  cvs_[65300uz - 1uz] = 0xCDu;  // CID low byte
  cvs_[65301uz - 1uz] = 0x2A;   // Session ID

  Expectation read_cv{EXPECT_CALL(*this, readCv(_))
                        .WillOnce(Return(cvs_[29uz - 1uz]))
                        .WillOnce(Return(cvs_[1uz - 1uz]))
                        .WillOnce(Return(cvs_[19uz - 1uz]))
                        .WillOnce(Return(cvs_[20uz - 1uz]))
                        .WillOnce(Return(cvs_[15uz - 1uz]))
                        .WillOnce(Return(cvs_[16uz - 1uz]))
                        .WillOnce(Return(cvs_[28uz - 1uz]))
                        .WillOnce(Return(cvs_[250uz - 1uz]))
                        .WillOnce(Return(cvs_[251uz - 1uz]))
                        .WillOnce(Return(cvs_[252uz - 1uz]))
                        .WillOnce(Return(cvs_[253uz - 1uz]))
                        .WillOnce(Return(cvs_[65297uz - 1uz]))
                        .WillOnce(Return(cvs_[65298uz - 1uz]))
                        .WillOnce(Return(cvs_[65299uz - 1uz]))
                        .WillOnce(Return(cvs_[65300uz - 1uz]))
                        .WillOnce(Return(cvs_[65301uz - 1uz]))};
  init();
}

BiDiTest::~BiDiTest() {}