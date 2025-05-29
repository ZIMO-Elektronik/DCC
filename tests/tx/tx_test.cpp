#include "tx_test.hpp"

bool TxTest::EvalPacket(dcc::tx::Timings timings,
                        dcc::Packet packet,
                        dcc::tx::Timings::size_type& offset,
                        dcc::tx::Config cfg) {
  if (!EvalPreamble(timings, offset, cfg)) return false;

  for (dcc::Packet::size_type i = 0; i < packet.size(); i++) {
    if (!EvalStartBit(timings, offset, cfg)) return false;
    if (!EvalByte(timings, packet[i], offset, cfg)) return false;
  }
  if (!EvalStopBit(timings, offset, cfg)) return false;
  if (!EvalBiDi(timings, offset, cfg)) return false;
  return true;
}

bool TxTest::EvalPreamble(dcc::tx::Timings timings,
                          dcc::tx::Timings::size_type& offset,
                          dcc::tx::Config cfg) {

  // Evaluate Preamble
  for (dcc::tx::Timings::size_type i = 0; i < cfg.num_preamble; i++) {
    if (timings[static_cast<dcc::tx::Timings::size_type>(2 * i)] !=
        timings[static_cast<dcc::tx::Timings::size_type>(2 * i + 1)])
      return false;
    if (timings[static_cast<dcc::tx::Timings::size_type>(2 * i)] !=
        cfg.bit1_duration)
      return false;
    offset += 2;
  }

  // Check start bit
  if (timings[static_cast<dcc::tx::Timings::size_type>(2 * cfg.num_preamble +
                                                       1)] == cfg.bit1_duration)
    return false;

  return true;
}

bool TxTest::EvalByte(dcc::tx::Timings timings,
                      dcc::Packet::value_type byte,
                      dcc::tx::Timings::size_type& offset,
                      dcc::tx::Config cfg) {
  uint8_t eval = 0u;
  bool res{true};

  for ([[maybe_unused]] int i = 7; i > -1; i--) {
    // Half-bits equal
    if (timings[static_cast<dcc::tx::Timings::size_type>(offset)] !=
        timings[static_cast<dcc::tx::Timings::size_type>(offset + 1)]) {
      res = false;
      break;
    }

    // Half-bits timing
    if (!((timings[static_cast<dcc::tx::Timings::size_type>(offset)] ==
           cfg.bit0_duration) ||
          (timings[static_cast<dcc::tx::Timings::size_type>(offset)] ==
           cfg.bit1_duration))) {
      res = false;
      break;
    }

    // Convert bit-halfs to bit
    if (timings[static_cast<dcc::tx::Timings::size_type>(offset)] ==
        cfg.bit1_duration)
      eval |= static_cast<uint8_t>(1u << i);

    offset += 2;
  }

  if (byte != eval) res = false;

  return res;
}

bool TxTest::EvalStartBit(dcc::tx::Timings timings,
                          dcc::tx::Timings::size_type& offset,
                          dcc::tx::Config cfg) {

  if ((timings[offset] != cfg.bit0_duration) ||
      (timings[offset + 1] != cfg.bit0_duration))
    return false;
  offset += 2;
  return true;
}

bool TxTest::EvalStopBit(dcc::tx::Timings timings,
                         dcc::tx::Timings::size_type& offset,
                         dcc::tx::Config cfg) {
  if ((timings[offset] != cfg.bit1_duration) ||
      (timings[offset + 1] != cfg.bit1_duration))
    return false;
  offset += 2;
  return true;
}

bool TxTest::EvalBiDi(dcc::tx::Timings timings,
                      dcc::tx::Timings::size_type& offset,
                      dcc::tx::Config cfg) {

  if (!cfg.flags.bidi) return true;

  std::array<uint16_t, 5> bidi{
    dcc::bidi::Timing::TCS,
    dcc::bidi::Timing::TTS1 - dcc::bidi::Timing::TCS,
    dcc::bidi::Timing::TTS2 - dcc::bidi::Timing::TTS1,
    dcc::bidi::Timing::TTC2 - dcc::bidi::Timing::TTS2,
    dcc::bidi::Timing::TCE - dcc::bidi::Timing::TTC2};

  for (uint16_t i = 0; i < bidi.size(); i++) {
    if (timings[offset++] != bidi[i]) return false;
  }
  return true;
}

dcc::tx::Timings TxTest::TransmitT(int lengthOfData, dcc::tx::Config cfg) {
  dcc::tx::Timings res{};

  // Preamble
  for (int i = 0; i < 2 * static_cast<int>(cfg.num_preamble); i++) {
    res.push_back(_mock.transmit());
  }

  // Data
  for (int i = 0; i < lengthOfData; i++) {
    for ([[maybe_unused]] int j = 0; j < 2 * 9; j++) {
      res.push_back(_mock.transmit());
    }
  }

  // Stop-Bit
  res.push_back(_mock.transmit());
  res.push_back(_mock.transmit());

  // BiDi
  if (cfg.flags.bidi)
    for (int i = 0; i < 5; i++) { res.push_back(_mock.transmit()); }

  return res;
}

dcc::tx::Timings TxTest::TransmitP(int lengthOfData, dcc::tx::Config cfg) {
  dcc::tx::Timings res{};

  // Preamble
  for (int i = 0; i < 2 * static_cast<int>(cfg.num_preamble); i++) {
    res.push_back(_mockp.transmit());
  }

  // Data
  for (int i = 0; i < lengthOfData; i++) {
    for ([[maybe_unused]] int j = 0; j < 2 * 9; j++) {
      res.push_back(_mockp.transmit());
    }
  }

  // Stop-Bit
  res.push_back(_mockp.transmit());
  res.push_back(_mockp.transmit());

  // BiDi
  if (cfg.flags.bidi)
    for (int i = 0; i < 5; i++) { res.push_back(_mockp.transmit()); }

  return res;
}

TxTest::TxTest() {}

TxTest::~TxTest() {}
