#include "tx_test.hpp"

TxTest::TxTest() {
  _packet_mock.init(_cfg);
  _timings_mock.init(_cfg);
}

TxTest::~TxTest() {}
