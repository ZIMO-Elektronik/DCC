#include "demo.hpp"
#include <dcc/dcc.hpp>
#include "utility.hpp"

#if defined(__STDCPP_FLOAT16_T__)
using FLOAT16 = std::float16_t;
#else
using FLOAT16 = float;
#endif

namespace {

void add_loco_packets(State& state) {
  // Decoder Control
  state.operations_packets.push_back(
    {.bytes = dcc::make_reset_packet(random_loco_address())});
  state.operations_packets.push_back(
    {.bytes = dcc::make_hard_reset_packet(random_loco_address())});
  state.operations_packets.push_back(
    {.bytes = dcc::make_factory_test_packet(random_loco_address(), true)});
  state.operations_packets.push_back(
    {.bytes =
       dcc::make_set_advanced_addressing_packet(random_loco_address(), true)});
  state.operations_packets.push_back(
    {.bytes = dcc::make_ack_request_packet(random_loco_address())});

  // Consist Control
  state.operations_packets.push_back(
    {.bytes = dcc::make_set_consist_address_packet(
       random_loco_address(), random_interval<uint8_t>())});

  // Advanced operations
  state.operations_packets.push_back(
    {.bytes = dcc::make_speed_direction_and_functions_packet(
       random_loco_address(),
       random_interval<uint8_t>(),
       random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = dcc::make_speed_direction_and_functions_packet(
       random_loco_address(),
       random_interval<uint8_t>(),
       random_interval<uint8_t>(),
       random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = dcc::make_speed_direction_and_functions_packet(
       random_loco_address(),
       random_interval<uint8_t>(),
       random_interval<uint8_t>(),
       random_interval<uint8_t>(),
       random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes =
       dcc::make_analog_function_group_packet(random_loco_address(),
                                              random_interval<uint8_t>(),
                                              random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = dcc::make_special_operating_modes(
       random_loco_address(),
       static_cast<dcc::Consist>(random_interval<uint8_t>(0b00u, 0b11u)),
       static_cast<bool>(random_interval(0, 1)),
       static_cast<bool>(random_interval(0, 1)),
       static_cast<bool>(random_interval(0, 1)),
       static_cast<bool>(random_interval(0, 1)))});
  state.operations_packets.push_back(
    {.bytes = dcc::make_128_speed_step_control_packet(
       random_loco_address(), random_interval<uint8_t>())});

  // Speed & direction
  state.operations_packets.push_back(
    {.bytes = dcc::make_speed_and_direction_packet(
       random_loco_address(),
       random_interval<uint8_t>(0b00'0000u, 0b11'1111u))});

  // Function group
  state.operations_packets.push_back(
    {.bytes = dcc::make_f0_f4_packet(random_loco_address(),
                                     random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = dcc::make_f9_f12_packet(random_loco_address(),
                                      random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = dcc::make_f5_f8_packet(random_loco_address(),
                                     random_interval<uint8_t>())});

  // Feature expansion
  state.operations_packets.push_back(
    {.bytes = dcc::make_binary_state_long_packet(
       random_loco_address(),
       random_interval<uint16_t>(),
       static_cast<bool>(random_interval(0, 1)))});
  state.operations_packets.push_back(
    {.bytes = dcc::make_time_packet(random_interval<uint8_t>(0u, 7u),
                                    random_interval<uint8_t>(0u, 23u),
                                    random_interval<uint8_t>(0u, 59u),
                                    random_interval<uint8_t>(0u, 63u),
                                    static_cast<bool>(random_interval(0, 1)))});
  state.operations_packets.push_back(
    {.bytes = dcc::make_date_packet(random_interval<uint8_t>(1u, 7u),
                                    random_interval<uint8_t>(1u, 12u),
                                    random_interval<uint16_t>(0u, 4095u))});
  state.operations_packets.push_back(
    {.bytes = dcc::make_time_scale_packet(
       static_cast<FLOAT16>(random_interval<float>(-100.0f, 100.0f)))});
  state.operations_packets.push_back(
    {.bytes = dcc::make_system_time_packet(random_interval<uint16_t>())});
  state.operations_packets.push_back(
    {.bytes = dcc::make_command_station_feature_identification_packet(
       dcc::LocoFeatures{random_interval<uint16_t>()})});
  state.operations_packets.push_back(
    {.bytes = dcc::make_command_station_feature_identification_packet(
       dcc::AccessoryBroadcastFeatures{random_interval<uint16_t>()})});
  state.operations_packets.push_back(
    {.bytes = dcc::make_command_station_feature_identification_packet(
       dcc::BiDiFeatures{random_interval<uint16_t>()})});
  state.operations_packets.push_back(
    {.bytes = dcc::make_f29_f36_packet(random_loco_address(),
                                       random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = dcc::make_f37_f44_packet(random_loco_address(),
                                       random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = dcc::make_f45_f52_packet(random_loco_address(),
                                       random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = dcc::make_f53_f60_packet(random_loco_address(),
                                       random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = dcc::make_f61_f68_packet(random_loco_address(),
                                       random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = dcc::make_binary_state_short_packet(
       random_loco_address(),
       random_interval<uint8_t>(),
       static_cast<bool>(random_interval(0, 1)))});
  state.operations_packets.push_back(
    {.bytes = dcc::make_f13_f20_packet(random_loco_address(),
                                       random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = dcc::make_f21_f28_packet(random_loco_address(),
                                       random_interval<uint8_t>())});

  // CV access
  state.operations_packets.push_back(
    {.bytes = dcc::make_cv_access_long_verify_packet(
       random_loco_address(),
       random_interval<uint16_t>(0u, smath::pow(2u, 10u) - 1u),
       random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = dcc::make_cv_access_long_write_packet(
       random_loco_address(),
       random_interval<uint16_t>(0u, smath::pow(2u, 10u) - 1u),
       random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = dcc::make_cv_access_long_verify_packet(
       random_loco_address(),
       random_interval<uint16_t>(0u, smath::pow(2u, 10u) - 1u),
       static_cast<bool>(random_interval(0, 1)),
       random_interval<uint8_t>(0u, 7u))});
  state.operations_packets.push_back(
    {.bytes = dcc::make_cv_access_long_write_packet(
       random_loco_address(),
       random_interval<uint16_t>(0u, smath::pow(2u, 10u) - 1u),
       static_cast<bool>(random_interval(0, 1)),
       random_interval<uint8_t>(0u, 7u))});
  state.operations_packets.push_back(
    {.bytes = make_cv_access_short_write_packet(
       random_loco_address(), 0b0010u, random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = make_cv_access_short_write_packet(
       random_loco_address(), 0b0011u, random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = make_cv_access_short_write_packet(random_loco_address(),
                                                0b0100u,
                                                random_interval<uint8_t>(),
                                                random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = make_cv_access_short_write_packet(random_loco_address(),
                                                0b0101u,
                                                random_interval<uint8_t>(),
                                                random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = make_cv_access_short_write_packet(random_loco_address(),
                                                0b0110u,
                                                random_interval<uint8_t>(),
                                                random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = make_cv_access_xpom_verify_packet(
       random_loco_address(),
       random_interval<uint8_t>(0u, 3u),
       random_interval<uint32_t>(0u, smath::pow(2u, 24u) - 1u))});
  state.operations_packets.push_back(
    {.bytes = make_cv_access_xpom_write_packet(
       random_loco_address(),
       random_interval<uint8_t>(0u, 3u),
       random_interval<uint32_t>(0u, smath::pow(2u, 24u) - 1u),
       random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = make_cv_access_xpom_write_packet(
       random_loco_address(),
       random_interval<uint8_t>(0u, 3u),
       random_interval<uint32_t>(0u, smath::pow(2u, 24u) - 1u),
       random_interval<uint8_t>(),
       random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = make_cv_access_xpom_write_packet(
       random_loco_address(),
       random_interval<uint8_t>(0u, 3u),
       random_interval<uint32_t>(0u, smath::pow(2u, 24u) - 1u),
       random_interval<uint8_t>(),
       random_interval<uint8_t>(),
       random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = make_cv_access_xpom_write_packet(
       random_loco_address(),
       random_interval<uint8_t>(0u, 3u),
       random_interval<uint32_t>(0u, smath::pow(2u, 24u) - 1u),
       random_interval<uint8_t>(),
       random_interval<uint8_t>(),
       random_interval<uint8_t>(),
       random_interval<uint8_t>())});
  state.operations_packets.push_back(
    {.bytes = make_cv_access_xpom_write_packet(
       random_loco_address(),
       random_interval<uint8_t>(0u, 3u),
       random_interval<uint32_t>(0u, smath::pow(2u, 24u) - 1u),
       static_cast<bool>(random_interval(0, 1)),
       random_interval<uint8_t>())});
}

//
void add_accessory_packets(State& state) {
  state.operations_packets.push_back(
    {.bytes = dcc::make_basic_accessory_packet(
       random_basic_accessory_address(),
       static_cast<bool>(random_interval(0, 1)),
       static_cast<bool>(random_interval(0, 1)))});
  state.operations_packets.push_back(
    {.bytes = dcc::make_extended_accessory_packet(
       random_extended_accessory_address(), random_interval<uint8_t>())});
  state.operations_packets.push_back({.bytes = dcc::make_accessory_nop_packet(
                                        random_basic_accessory_address())});
  state.operations_packets.push_back({.bytes = dcc::make_accessory_nop_packet(
                                        random_extended_accessory_address())});
}

//
void add_service_packets(State& state) {
  state.service_packets.push_back(
    {.bytes = dcc::make_cv_access_long_verify_service_packet(
       random_interval<uint16_t>(0u, smath::pow(2u, 10u) - 1u),
       random_interval<uint8_t>())});
  state.service_packets.push_back(
    {.bytes = dcc::make_cv_access_long_write_service_packet(
       random_interval<uint16_t>(0u, smath::pow(2u, 10u) - 1u),
       random_interval<uint8_t>())});
  state.service_packets.push_back(
    {.bytes = dcc::make_cv_access_long_verify_service_packet(
       random_interval<uint16_t>(0u, smath::pow(2u, 10u) - 1u),
       static_cast<bool>(random_interval(0, 1)),
       random_interval<uint8_t>(0u, 7u))});
  state.service_packets.push_back(
    {.bytes = dcc::make_cv_access_long_write_service_packet(
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
