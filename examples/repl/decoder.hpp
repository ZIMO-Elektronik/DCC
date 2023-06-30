#pragma once

#include <dcc/dcc.hpp>

struct Decoder : dcc::rx::CrtpBase<Decoder> {
  friend dcc::rx::CrtpBase<Decoder>;
  friend dcc::rx::CrtpBase<Decoder>::BiDi;

  Decoder();

private:
  // Set direction (1 forward, -1 backward)
  void direction(uint32_t addr, int32_t dir);

  // Set notch (0-255)
  void notch(uint32_t addr, int32_t notch);

  // Emergency stop
  void emergencyStop(uint32_t addr);

  // Set function inputs
  void function(uint32_t addr, uint32_t mask, uint32_t state);

  // Enter or exit service mode
  void serviceModeHook(bool service_mode);

  // Generate current pulse as service ACK
  void serviceAck();

  // Transmit BiDi
  void transmitBiDi(std::span<uint8_t const> chunk);

  // Read CV
  uint8_t readCv(uint32_t cv_addr, uint8_t byte = 0u);

  // Write CV
  uint8_t writeCv(uint32_t cv_addr, uint8_t byte);

  // Read CV bit
  bool readCv(uint32_t cv_addr, bool bit, uint32_t pos);

  // Write CV bit
  bool writeCv(uint32_t cv_addr, bool bit, uint32_t pos);

  std::array<uint8_t, 1024uz> _cvs{};
};
