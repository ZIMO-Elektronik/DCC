#include "decoder.hpp"
#include <climits>
#include <cstdio>
#include "bsp.h"

void Decoder::direction(uint16_t addr, bool dir) {}

void Decoder::speed(uint16_t addr, int32_t speed) {
  if (speed) {
    printf("\nDecoder: accelerate to speed step %d\n", speed);
    bsp_write_green_led(true);
  } else {
    printf("Decoder: stop\n");
    bsp_write_green_led(false);
  }
}

void Decoder::function(uint16_t addr, uint32_t mask, uint32_t state) {
  if (!(mask & 0b0'1000u)) return;
  else if (state & 0b0'1000u) {
    printf("Decoder: set function F3\n");
    bsp_write_yellow_led(true);
  } else {
    printf("Decoder: clear function F3\n");
    bsp_write_yellow_led(false);
  }
}

void Decoder::serviceModeHook(bool service_mode) {}

void Decoder::serviceAck() {}

void Decoder::transmitBiDi(std::span<uint8_t const> bytes) {}

uint8_t Decoder::readCv(uint32_t cv_addr, uint8_t) {
  if (cv_addr >= size(_cvs)) return 0u;
  return _cvs[cv_addr];
}

uint8_t Decoder::writeCv(uint32_t cv_addr, uint8_t byte) {
  if (cv_addr >= size(_cvs)) return 0u;
  return _cvs[cv_addr] = byte;
}

bool Decoder::readCv(uint32_t cv_addr, bool, uint32_t pos) { return false; }

bool Decoder::writeCv(uint32_t cv_addr, bool bit, uint32_t pos) {
  return false;
}

Decoder decoder;

extern "C" void TIMER_IRQ_HANDLER() {
  auto const ccr{bsp_decoder_irq()};
  decoder.receive(ccr);
}

int main() {
  bsp_init_decoder();
  decoder.init();

  printf("\n\nBoot\n");
  for (;;) {
    decoder.execute();
    bsp_delay(5u);
  }
}
