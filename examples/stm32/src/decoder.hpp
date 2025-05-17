#pragma once

#include <dcc/dcc.hpp>

struct Decoder : dcc::rx::CrtpBase<Decoder> {
  friend dcc::rx::CrtpBase<Decoder>;

private:
  // Set direction (1 forward, 0 backward)
  void direction(uint16_t addr, bool dir);

  // Set speed [-1, 255] (regardless of CV settings)
  void speed(uint16_t addr, int32_t speed);

  // Set function inputs
  void function(uint16_t addr, uint32_t mask, uint32_t state);

  // Enter or exit service mode
  void serviceModeHook(bool service_mode);

  // Generate current pulse as service ACK
  void serviceAck();

  // Transmit BiDi
  void transmitBiDi(std::span<uint8_t const> bytes);

  // Read CV
  uint8_t readCv(uint32_t cv_addr, uint8_t byte = 0u);

  // Write CV
  uint8_t writeCv(uint32_t cv_addr, uint8_t byte);

  // Read CV bit
  bool readCv(uint32_t cv_addr, bool, uint32_t pos);

  // Write CV bit
  bool writeCv(uint32_t cv_addr, bool bit, uint32_t pos);

  // Minimal set of CVs
  std::array<uint8_t, 29uz> _cvs{
    3u,   1u,   2u, 1u,   1u,   1u,   4u, DCC_MANUFACTURER_ID,
    55u,  0u,   0u, 117u, 128u, 195u, 0u, 0u,
    192u, 128u, 0u, 0u,   0u,   0u,   0u, 0u,
    0u,   0u,   0u, 131u, 14u};
};
