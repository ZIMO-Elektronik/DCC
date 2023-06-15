#include "decoder.hpp"
#include <cli/cli.h>

// Make the prompt look nice again after output
#define PROMPTENDL                                                             \
  "\ndcc> ";                                                                   \
  std::cout << std::flush

Decoder::Decoder() {
  cvs_[29uz - 1uz] = 0b10u;  // Decoder configuration
  cvs_[1uz - 1uz] = 3u;      // Primary address
  cvs_[8uz - 1uz] = 145u;    // ZIMO manufacturer ID
};

void Decoder::direction(uint32_t addr, int32_t dir) {
  cli::Cli::cout() << "Address " << addr << ": set direction "
                   << (dir == 1 ? "forward" : "backward") << PROMPTENDL;
}

void Decoder::notch(uint32_t addr, int32_t notch) {
  cli::Cli::cout() << "Address " << addr << ": set speed " << notch
                   << PROMPTENDL;
}

void Decoder::emergencyStop(uint32_t addr) {
  cli::Cli::cout() << "Address " << addr << ": e-stop" << PROMPTENDL;
}

void Decoder::function(uint32_t addr, uint32_t mask, uint32_t state) {
  auto const f_high{std::bit_width(mask) - 1u};
  auto const f_low{std::countr_zero(mask)};
  cli::Cli::cout() << "Address " << addr << ": f" << f_high << "-"
                   << "f" << f_low << " = "
                   << "0b";
  for (int i{f_high}; i >= f_low; --i)
    cli::Cli::cout() << static_cast<bool>(state & (1u << i));
  cli::Cli::cout() << PROMPTENDL;
}

void Decoder::serviceModeHook(bool) {}

void Decoder::serviceAck() {}

void Decoder::transmitBiDi(std::span<uint8_t const>) {}

uint8_t Decoder::readCv(uint32_t cv_addr, uint8_t byte) {
  cli::Cli::cout() << "Read CV byte " << cv_addr
                   << "==" << static_cast<uint32_t>(cvs_[cv_addr])
                   << PROMPTENDL;
  return cvs_[cv_addr];
}

uint8_t Decoder::writeCv(uint32_t cv_addr, uint8_t byte) {
  cli::Cli::cout() << "Write CV byte " << cv_addr << "="
                   << static_cast<uint32_t>(byte) << PROMPTENDL;
  return cvs_[cv_addr] = byte;
}

bool Decoder::readCv(uint32_t cv_addr, bool bit, uint32_t pos) {
  auto const red_bit{static_cast<bool>(cvs_[cv_addr] & (1u << pos))};
  cli::Cli::cout() << "Read CV bit " << cv_addr << ":" << pos << "==" << red_bit
                   << PROMPTENDL;
  return red_bit;
}

bool Decoder::writeCv(uint32_t cv_addr, bool bit, uint32_t pos) {
  cvs_[cv_addr] = (cvs_[cv_addr] & ~(1u << pos)) | (bit << pos);
  auto const red_bit{static_cast<bool>(cvs_[cv_addr] & (1u << pos))};
  cli::Cli::cout() << "Write CV bit " << cv_addr << ":" << pos << "=" << red_bit
                   << PROMPTENDL;
  return red_bit;
}
