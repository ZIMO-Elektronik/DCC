#include "decoder.hpp"
#include <cli/cli.h>

// Make the prompt look nice again after output
#define PROMPTENDL                                                             \
  "\ndcc> ";                                                                   \
  std::cout << std::flush

Decoder::Decoder() {
  _cvs[29uz - 1uz] = 0b10u;              // Decoder configuration
  _cvs[1uz - 1uz] = 3u;                  // Primary address
  _cvs[8uz - 1uz] = DCC_MANUFACTURER_ID; // Manufacturer ID
}

void Decoder::direction(uint16_t addr, bool dir) {
  cli::Cli::cout() << "Address " << addr << ": set direction "
                   << (dir ? "forward" : "backward") << PROMPTENDL;
}

void Decoder::speed(uint16_t addr, int32_t speed) {
  cli::Cli::cout() << "Address " << addr << ": set speed " << speed
                   << PROMPTENDL;
}

void Decoder::function(uint16_t addr, uint32_t mask, uint32_t state) {
  auto const f_high{std::bit_width(mask) - 1};
  auto const f_low{std::countr_zero(mask)};
  cli::Cli::cout() << "Address " << addr << ": f" << f_high << "-"
                   << "f" << f_low << " = "
                   << "0b";
  for (auto i{f_high}; i >= f_low; --i)
    cli::Cli::cout() << static_cast<bool>(state & (1u << i));
  cli::Cli::cout() << PROMPTENDL;
}

void Decoder::serviceModeHook(bool) {}

void Decoder::serviceAck() {}

void Decoder::transmitBiDi(std::span<uint8_t const>) {}

uint8_t Decoder::readCv(uint32_t cv_addr, [[maybe_unused]] uint8_t byte) {
  auto const red_byte{
    static_cast<uint8_t>(cv_addr < size(_cvs) ? _cvs[cv_addr] : 0u)};
  cli::Cli::cout() << "Read CV byte " << cv_addr
                   << "==" << static_cast<uint32_t>(red_byte) << PROMPTENDL;
  return red_byte;
}

uint8_t Decoder::writeCv(uint32_t cv_addr, uint8_t byte) {
  cli::Cli::cout() << "Write CV byte " << cv_addr << "="
                   << static_cast<uint32_t>(byte) << PROMPTENDL;
  return _cvs[cv_addr] = byte;
}

bool Decoder::readCv(uint32_t cv_addr,
                     [[maybe_unused]] bool bit,
                     uint32_t pos) {
  auto const red_bit{static_cast<bool>(_cvs[cv_addr] & (1u << pos))};
  cli::Cli::cout() << "Read CV bit " << cv_addr << ":" << pos << "==" << red_bit
                   << PROMPTENDL;
  return red_bit;
}

bool Decoder::writeCv(uint32_t cv_addr, bool bit, uint32_t pos) {
  _cvs[cv_addr] =
    static_cast<uint8_t>((_cvs[cv_addr] & ~(1u << pos)) | (bit << pos));
  auto const red_bit{static_cast<bool>(_cvs[cv_addr] & (1u << pos))};
  cli::Cli::cout() << "Write CV bit " << cv_addr << ":" << pos << "=" << red_bit
                   << PROMPTENDL;
  return red_bit;
}
