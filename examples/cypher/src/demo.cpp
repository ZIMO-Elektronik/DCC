#include "demo.hpp"
#include <dcc/dcc.hpp>
#include "utility.hpp"

namespace {

void add_packets(State& state) {
  // Decoder Control
  state.packets.push_back(
    {.bytes = dcc::make_reset_packet(random_loco_address())});
  state.packets.push_back(
    {.bytes = dcc::make_hard_reset_packet(random_loco_address())});
  state.packets.push_back(
    {.bytes = dcc::make_factory_test_packet(random_loco_address(), true)});
  state.packets.push_back({.bytes = dcc::make_set_advanced_addressing_packet(
                             random_loco_address(), true)});
  state.packets.push_back(
    {.bytes = dcc::make_ack_request_packet(random_loco_address())});

  // Consist Control
  state.packets.push_back(
    {.bytes = dcc::make_set_consist_address_packet(
       random_loco_address(), random_interval<uint8_t>())});

  // Advanced operations
  state.packets.push_back(
    {.bytes = dcc::make_speed_direction_and_functions_packet(
       random_loco_address(),
       random_interval<uint8_t>(),
       random_interval<uint8_t>())});
  state.packets.push_back(
    {.bytes = dcc::make_speed_direction_and_functions_packet(
       random_loco_address(),
       random_interval<uint8_t>(),
       random_interval<uint8_t>(),
       random_interval<uint8_t>())});
  state.packets.push_back(
    {.bytes = dcc::make_speed_direction_and_functions_packet(
       random_loco_address(),
       random_interval<uint8_t>(),
       random_interval<uint8_t>(),
       random_interval<uint8_t>(),
       random_interval<uint8_t>())});
  state.packets.push_back({.bytes = dcc::make_analog_function_group_packet(
                             random_loco_address(),
                             random_interval<uint8_t>(),
                             random_interval<uint8_t>())});
  state.packets.push_back(
    {.bytes = dcc::make_special_operating_modes(
       random_loco_address(),
       static_cast<dcc::Consist>(random_interval<uint8_t>(0b00u, 0b11u)),
       random_interval(0, 1),
       random_interval(0, 1),
       random_interval(0, 1),
       random_interval(0, 1))});
  state.packets.push_back(
    {.bytes = dcc::make_128_speed_step_control_packet(
       random_loco_address(), random_interval<uint8_t>())});

  // Speed & direction
  state.packets.push_back(
    {.bytes = dcc::make_speed_and_direction_packet(
       random_loco_address(),
       random_interval<uint8_t>(0b00'0000u, 0b11'1111u))});

  // Function group
  state.packets.push_back(
    {.bytes = dcc::make_f0_f4_packet(random_loco_address(),
                                     random_interval<uint8_t>())});
  state.packets.push_back(
    {.bytes = dcc::make_f9_f12_packet(random_loco_address(),
                                      random_interval<uint8_t>())});
  state.packets.push_back(
    {.bytes = dcc::make_f5_f8_packet(random_loco_address(),
                                     random_interval<uint8_t>())});

  // Feature expansion
  state.packets.push_back(
    {.bytes = dcc::make_binary_state_long_packet(random_loco_address(),
                                                 random_interval<uint16_t>(),
                                                 random_interval(0, 1))});
  state.packets.push_back(
    {.bytes = dcc::make_time_packet(random_interval<uint8_t>(0u, 7u),
                                    random_interval<uint8_t>(0u, 23u),
                                    random_interval<uint8_t>(0u, 59u),
                                    random_interval<uint8_t>(0u, 63u),
                                    random_interval(0, 1))});
  state.packets.push_back(
    {.bytes = dcc::make_date_packet(random_interval<uint8_t>(1u, 7u),
                                    random_interval<uint8_t>(1u, 12u),
                                    random_interval<uint16_t>(0u, 4095u))});
  state.packets.push_back(
    {.bytes = dcc::make_system_time_packet(random_interval<uint16_t>())});
  // Command station properties identifier
  state.packets.push_back(
    {.bytes = dcc::make_f29_f36_packet(random_loco_address(),
                                       random_interval<uint8_t>())});
  state.packets.push_back(
    {.bytes = dcc::make_f37_f44_packet(random_loco_address(),
                                       random_interval<uint8_t>())});
  state.packets.push_back(
    {.bytes = dcc::make_f45_f52_packet(random_loco_address(),
                                       random_interval<uint8_t>())});
  state.packets.push_back(
    {.bytes = dcc::make_f53_f60_packet(random_loco_address(),
                                       random_interval<uint8_t>())});
  state.packets.push_back(
    {.bytes = dcc::make_f61_f68_packet(random_loco_address(),
                                       random_interval<uint8_t>())});
  state.packets.push_back(
    {.bytes = dcc::make_binary_state_short_packet(random_loco_address(),
                                                  random_interval<uint8_t>(),
                                                  random_interval(0, 1))});
  state.packets.push_back(
    {.bytes = dcc::make_f13_f20_packet(random_loco_address(),
                                       random_interval<uint8_t>())});
  state.packets.push_back(
    {.bytes = dcc::make_f21_f28_packet(random_loco_address(),
                                       random_interval<uint8_t>())});
  // CV long
  // CV short
}

void add_datagrams(State& state) {}

} // namespace

void demo(State& state) {
  add_packets(state);
  add_datagrams(state);
}
