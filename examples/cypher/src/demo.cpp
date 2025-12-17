#include "demo.hpp"
#include <dcc/dcc.hpp>
#include "utility.hpp"

namespace {

void add_loco_packets(State& state) {
  // Decoder Control
  auto addr{random_loco_address()};
  state.packets.push_back({.addr = addr, .bytes = make_reset_packet(addr)});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr, .bytes = make_hard_reset_packet(addr)});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr, .bytes = make_factory_test_packet(addr, true)});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr, .bytes = make_set_advanced_addressing_packet(addr, true)});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr, .bytes = make_ack_request_packet(addr)});

  // Consist Control
  addr = random_loco_address();
  state.packets.push_back({.addr = addr,
                           .bytes = make_set_consist_address_packet(
                             addr, random_interval<uint8_t>())});

  // Advanced operations
  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_speed_direction_and_functions_packet(
       addr, random_interval<uint8_t>(), random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back({.addr = addr,
                           .bytes = make_speed_direction_and_functions_packet(
                             addr,
                             random_interval<uint8_t>(),
                             random_interval<uint8_t>(),
                             random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back({.addr = addr,
                           .bytes = make_speed_direction_and_functions_packet(
                             addr,
                             random_interval<uint8_t>(),
                             random_interval<uint8_t>(),
                             random_interval<uint8_t>(),
                             random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_analog_function_group_packet(
       addr, random_interval<uint8_t>(), random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_special_operating_modes(
       addr,
       static_cast<dcc::Consist>(random_interval<uint8_t>(0b00u, 0b11u)),
       static_cast<bool>(random_interval(0, 1)),
       static_cast<bool>(random_interval(0, 1)),
       static_cast<bool>(random_interval(0, 1)),
       static_cast<bool>(random_interval(0, 1)))});

  addr = random_loco_address();
  state.packets.push_back({.addr = addr,
                           .bytes = make_128_speed_step_control_packet(
                             addr, random_interval<uint8_t>())});

  // Speed & direction
  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_speed_and_direction_packet(
       addr, random_interval<uint8_t>(0b00'0000u, 0b11'1111u))});

  // Function group
  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_f0_f4_packet(addr, random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_f9_f12_packet(addr, random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_f5_f8_packet(addr, random_interval<uint8_t>())});

  // Feature expansion
  addr = random_loco_address();
  state.packets.push_back({.addr = addr,
                           .bytes = make_binary_state_long_packet(
                             addr,
                             random_interval<uint16_t>(),
                             static_cast<bool>(random_interval(0, 1)))});

  state.packets.push_back(
    {.addr = {.type = dcc::Address::Broadcast},
     .bytes = dcc::make_time_packet(random_interval<uint8_t>(0u, 7u),
                                    random_interval<uint8_t>(0u, 23u),
                                    random_interval<uint8_t>(0u, 59u),
                                    random_interval<uint8_t>(0u, 63u),
                                    static_cast<bool>(random_interval(0, 1)))});

  state.packets.push_back(
    {.addr = {.type = dcc::Address::Broadcast},
     .bytes = dcc::make_date_packet(random_interval<uint8_t>(1u, 7u),
                                    random_interval<uint8_t>(1u, 12u),
                                    random_interval<uint16_t>(0u, 4095u))});

  state.packets.push_back(
    {.addr = {.type = dcc::Address::Broadcast},
     .bytes = dcc::make_time_scale_packet(
       static_cast<FLOAT16>(random_interval<float>(-100.0f, 100.0f)))});

  state.packets.push_back(
    {.addr = {.type = dcc::Address::Broadcast},
     .bytes = dcc::make_system_time_packet(random_interval<uint16_t>())});

  state.packets.push_back(
    {.addr = {.type = dcc::Address::Broadcast},
     .bytes = dcc::make_command_station_feature_identification_packet(
       dcc::LocoFeatures{random_interval<uint16_t>()})});

  state.packets.push_back(
    {.addr = {.type = dcc::Address::Broadcast},
     .bytes = dcc::make_command_station_feature_identification_packet(
       dcc::AccessoryBroadcastFeatures{random_interval<uint16_t>()})});

  state.packets.push_back(
    {.addr = {.type = dcc::Address::Broadcast},
     .bytes = dcc::make_command_station_feature_identification_packet(
       dcc::BiDiFeatures{random_interval<uint16_t>()})});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_f29_f36_packet(addr, random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_f37_f44_packet(addr, random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_f45_f52_packet(addr, random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_f53_f60_packet(addr, random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_f61_f68_packet(addr, random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back({.addr = addr,
                           .bytes = make_binary_state_short_packet(
                             addr,
                             random_interval<uint8_t>(),
                             static_cast<bool>(random_interval(0, 1)))});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_f13_f20_packet(addr, random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_f21_f28_packet(addr, random_interval<uint8_t>())});

  // CV access
  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_cv_access_long_verify_packet(
       addr,
       random_interval<uint16_t>(0u, smath::pow(2u, 10u) - 1u),
       random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_cv_access_long_write_packet(
       addr,
       random_interval<uint16_t>(0u, smath::pow(2u, 10u) - 1u),
       random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_cv_access_long_verify_packet(
       addr,
       random_interval<uint16_t>(0u, smath::pow(2u, 10u) - 1u),
       static_cast<bool>(random_interval(0, 1)),
       random_interval<uint8_t>(0u, 7u))});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_cv_access_long_write_packet(
       addr,
       random_interval<uint16_t>(0u, smath::pow(2u, 10u) - 1u),
       static_cast<bool>(random_interval(0, 1)),
       random_interval<uint8_t>(0u, 7u))});

  addr = random_loco_address();
  state.packets.push_back({.addr = addr,
                           .bytes = make_cv_access_short_write_packet(
                             addr, 0b0010u, random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back({.addr = addr,
                           .bytes = make_cv_access_short_write_packet(
                             addr, 0b0011u, random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_cv_access_short_write_packet(
       addr, 0b0100u, random_interval<uint8_t>(), random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_cv_access_short_write_packet(
       addr, 0b0101u, random_interval<uint8_t>(), random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_cv_access_short_write_packet(
       addr, 0b0110u, random_interval<uint8_t>(), random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_cv_access_xpom_verify_packet(
       addr,
       random_interval<uint8_t>(0u, 3u),
       random_interval<uint32_t>(0u, smath::pow(2u, 24u) - 1u))});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_cv_access_xpom_write_packet(
       addr,
       random_interval<uint8_t>(0u, 3u),
       random_interval<uint32_t>(0u, smath::pow(2u, 24u) - 1u),
       random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_cv_access_xpom_write_packet(
       addr,
       random_interval<uint8_t>(0u, 3u),
       random_interval<uint32_t>(0u, smath::pow(2u, 24u) - 1u),
       random_interval<uint8_t>(),
       random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_cv_access_xpom_write_packet(
       addr,
       random_interval<uint8_t>(0u, 3u),
       random_interval<uint32_t>(0u, smath::pow(2u, 24u) - 1u),
       random_interval<uint8_t>(),
       random_interval<uint8_t>(),
       random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_cv_access_xpom_write_packet(
       addr,
       random_interval<uint8_t>(0u, 3u),
       random_interval<uint32_t>(0u, smath::pow(2u, 24u) - 1u),
       random_interval<uint8_t>(),
       random_interval<uint8_t>(),
       random_interval<uint8_t>(),
       random_interval<uint8_t>())});

  addr = random_loco_address();
  state.packets.push_back(
    {.addr = addr,
     .bytes = make_cv_access_xpom_write_packet(
       addr,
       random_interval<uint8_t>(0u, 3u),
       random_interval<uint32_t>(0u, smath::pow(2u, 24u) - 1u),
       static_cast<bool>(random_interval(0, 1)),
       random_interval<uint8_t>())});

  state.packets.push_back({.addr = {.value = 0xFFu, .type = dcc::Address::Idle},
                           .bytes = dcc::make_idle_packet()});
}

//
void add_accessory_packets(State& state) {
  auto addr{random_basic_accessory_address()};
  state.packets.push_back({.addr = addr,
                           .bytes = make_basic_accessory_packet(
                             addr,
                             static_cast<bool>(random_interval(0, 1)),
                             static_cast<bool>(random_interval(0, 1)))});

  addr = random_basic_accessory_address();
  state.packets.push_back(
    {.addr = addr, .bytes = make_accessory_nop_packet(addr)});

  addr = random_extended_accessory_address();
  state.packets.push_back({.addr = addr,
                           .bytes = make_extended_accessory_packet(
                             addr, random_interval<uint8_t>())});

  addr = random_extended_accessory_address();
  state.packets.push_back(
    {.addr = addr, .bytes = make_accessory_nop_packet(addr)});
}

//
void add_service_packets(State& state) {
  state.packets.push_back(
    {.addr = {.type = dcc::Address::UnknownService},
     .bytes = dcc::make_cv_access_long_verify_service_packet(
       random_interval<uint16_t>(0u, smath::pow(2u, 10u) - 1u),
       random_interval<uint8_t>())});
  state.packets.push_back(
    {.addr = {.type = dcc::Address::UnknownService},
     .bytes = dcc::make_cv_access_long_write_service_packet(
       random_interval<uint16_t>(0u, smath::pow(2u, 10u) - 1u),
       random_interval<uint8_t>())});
  state.packets.push_back(
    {.addr = {.type = dcc::Address::UnknownService},
     .bytes = dcc::make_cv_access_long_verify_service_packet(
       random_interval<uint16_t>(0u, smath::pow(2u, 10u) - 1u),
       static_cast<bool>(random_interval(0, 1)),
       random_interval<uint8_t>(0u, 7u))});
  state.packets.push_back(
    {.addr = {.type = dcc::Address::UnknownService},
     .bytes = dcc::make_cv_access_long_write_service_packet(
       random_interval<uint16_t>(0u, smath::pow(2u, 10u) - 1u),
       static_cast<bool>(random_interval(0, 1)),
       random_interval<uint8_t>(0u, 7u))});
}

//
void add_loco_datagrams(State& state) {}

//
void add_accessory_datagrams(State& state) {}

} // namespace

void demo(State& state) {
  add_loco_packets(state);
  add_accessory_packets(state);
  add_service_packets(state);
  add_loco_datagrams(state);
  add_accessory_datagrams(state);
}
