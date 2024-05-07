#include "command_station.hpp"
#include <cstdio>
#include "bsp.h"

void CommandStation::trackOutputs(bool N, bool P) { bsp_write_track(N, P); }

void CommandStation::biDiStart() {}

void CommandStation::biDiChannel1() {}

void CommandStation::biDiChannel2() {}

void CommandStation::biDiEnd() {}

CommandStation command_station;

extern "C" void TIMER_IRQ_HANDLER() {
  auto const arr{command_station.transmit()};
  bsp_command_station_irq(arr);
}

int main() {
  bsp_init_command_station();
  command_station.init({
    .num_preamble = DCC_TX_MIN_PREAMBLE_BITS,
    .bit1_duration = 58u,
    .bit0_duration = 100u,
    .flags = {.invert = false, .bidi = true},
  });

  // Turn red LED on to indicate this board is the command station
  bsp_write_red_led(true);

  printf("\n\nBoot\n");
  bsp_delay(2000u);
  dcc::Packet packet{};
  for (;;) {
    // Accelerate
    packet = dcc::make_advanced_operations_speed_packet(3u, 1u << 7u | 42u);
    command_station.packet(packet);
    printf("\nCommand station: accelerate to speed step 42\n");
    bsp_write_green_led(true);
    bsp_delay(2000u);

    // Set function F3
    packet = dcc::make_function_group_f4_f0_packet(3u, 0b0'1000u);
    command_station.packet(packet);
    printf("Command station: set function F3\n");
    bsp_write_yellow_led(true);
    bsp_delay(2000u);

    // Decelerate
    packet = dcc::make_advanced_operations_speed_packet(3u, 1u << 7u | 0u);
    command_station.packet(packet);
    printf("Command station: stop\n");
    bsp_write_green_led(false);
    bsp_delay(2000u);

    // Clear function
    packet = dcc::make_function_group_f4_f0_packet(3u, 0b0'0000u);
    command_station.packet(packet);
    printf("Command station: clear function F3\n");
    bsp_write_yellow_led(false);
    bsp_delay(2000u);
  }
}
