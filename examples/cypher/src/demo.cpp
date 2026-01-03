#include "demo.hpp"
#include <dcc/dcc.hpp>
#include "state.hpp"
#include "utility.hpp"

namespace {

using namespace dcc;
using namespace dcc::bidi;

// Add loco packets
void add_loco_packets() {
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
       static_cast<Consist>(random_interval<uint8_t>(0b00u, 0b11u)),
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
    {.addr = {.type = Address::Broadcast},
     .bytes = make_time_packet(random_interval<uint8_t>(0u, 7u),
                               random_interval<uint8_t>(0u, 23u),
                               random_interval<uint8_t>(0u, 59u),
                               random_interval<uint8_t>(0u, 63u),
                               static_cast<bool>(random_interval(0, 1)))});

  state.packets.push_back(
    {.addr = {.type = Address::Broadcast},
     .bytes = make_date_packet(random_interval<uint8_t>(1u, 7u),
                               random_interval<uint8_t>(1u, 12u),
                               random_interval<uint16_t>(0u, 4095u))});

  state.packets.push_back({.addr = {.type = Address::Broadcast},
                           .bytes = make_time_scale_packet(static_cast<FLOAT16>(
                             random_interval<float>(-100.0f, 100.0f)))});

  state.packets.push_back(
    {.addr = {.type = Address::Broadcast},
     .bytes = make_system_time_packet(random_interval<uint16_t>())});

  state.packets.push_back(
    {.addr = {.type = Address::Broadcast},
     .bytes = make_command_station_feature_identification_packet(
       LocoFeatures{random_interval<uint16_t>()})});

  state.packets.push_back(
    {.addr = {.type = Address::Broadcast},
     .bytes = make_command_station_feature_identification_packet(
       AccessoryBroadcastFeatures{random_interval<uint16_t>()})});

  state.packets.push_back(
    {.addr = {.type = Address::Broadcast},
     .bytes = make_command_station_feature_identification_packet(
       BiDiFeatures{random_interval<uint16_t>()})});

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

  state.packets.push_back({.addr = {.value = 0xFFu, .type = Address::Idle},
                           .bytes = make_idle_packet()});
}

// Add accessory packets
void add_accessory_packets() {
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

// Add service packets
void add_service_packets() {
  state.packets.push_back(
    {.addr = {.type = Address::UnknownService},
     .bytes = make_cv_access_long_verify_service_packet(
       random_interval<uint16_t>(0u, smath::pow(2u, 10u) - 1u),
       random_interval<uint8_t>())});
  state.packets.push_back(
    {.addr = {.type = Address::UnknownService},
     .bytes = make_cv_access_long_write_service_packet(
       random_interval<uint16_t>(0u, smath::pow(2u, 10u) - 1u),
       random_interval<uint8_t>())});
  state.packets.push_back(
    {.addr = {.type = Address::UnknownService},
     .bytes = make_cv_access_long_verify_service_packet(
       random_interval<uint16_t>(0u, smath::pow(2u, 10u) - 1u),
       static_cast<bool>(random_interval(0, 1)),
       random_interval<uint8_t>(0u, 7u))});
  state.packets.push_back(
    {.addr = {.type = Address::UnknownService},
     .bytes = make_cv_access_long_write_service_packet(
       random_interval<uint16_t>(0u, smath::pow(2u, 10u) - 1u),
       static_cast<bool>(random_interval(0, 1)),
       random_interval<uint8_t>(0u, 7u))});
}

// Add ACK/NAK datagrams
void add_ack_nak_datagrams() {
  // ACK
  {
    State::Datagram datagram{.addr = random_loco_address()};
    datagram.bytes[channel1_size] = dcc::bidi::acks[0uz];
    state.datagrams.push_back(datagram);
  }
  {
    State::Datagram datagram{.addr = random_loco_address()};
    datagram.bytes[channel1_size] = dcc::bidi::acks[1uz];
    state.datagrams.push_back(datagram);
  }

  // NAK
  {
    State::Datagram datagram{.addr = random_loco_address()};
    datagram.bytes[channel1_size] = dcc::bidi::nak;
    state.datagrams.push_back(datagram);
  }
}

// Add loco datagrams
void add_loco_datagrams() {
  // app:pom
  {
    State::Datagram datagram{.addr = random_loco_address()};
    std::ranges::copy(make_app_pom_datagram(random_interval<uint8_t>()),
                      begin(datagram.bytes));
    state.datagrams.push_back(datagram);
  }

  // app:adr_high
  {
    State::Datagram datagram{.addr = random_basic_loco_address()};
    std::ranges::copy(make_app_adr_high_datagram(datagram.addr, 0u),
                      begin(datagram.bytes));
    state.datagrams.push_back(datagram);
  }
  {
    State::Datagram datagram{.addr = random_basic_loco_address()};
    std::ranges::copy(make_app_adr_high_datagram(datagram.addr, 1u),
                      begin(datagram.bytes));
    state.datagrams.push_back(datagram);
  }
  {
    State::Datagram datagram{.addr = random_extended_loco_address()};
    std::ranges::copy(make_app_adr_high_datagram(datagram.addr, 0u),
                      begin(datagram.bytes));
    state.datagrams.push_back(datagram);
  }

  // app:adr_low
  {
    State::Datagram datagram{.addr = random_basic_loco_address()};
    std::ranges::copy(make_app_adr_low_datagram(datagram.addr, 0u),
                      begin(datagram.bytes));
    state.datagrams.push_back(datagram);
  }
  {
    State::Datagram datagram{.addr = random_basic_loco_address()};
    std::ranges::copy(make_app_adr_low_datagram(datagram.addr, 1u),
                      begin(datagram.bytes));
    state.datagrams.push_back(datagram);
  }
  {
    State::Datagram datagram{.addr = random_extended_loco_address()};
    std::ranges::copy(make_app_adr_low_datagram(datagram.addr, 0u),
                      begin(datagram.bytes));
    state.datagrams.push_back(datagram);
  }

  // app:info1
  {
    State::Datagram datagram{.addr = random_loco_address()};
    std::ranges::copy(make_app_info1_datagram({static_cast<app::Info1::Flags>(
                        random_interval<uint8_t>())}),
                      begin(datagram.bytes));
    state.datagrams.push_back(datagram);
  }

  // app:ext
  {
    State::Datagram datagram{.addr = random_loco_address()};
    std::ranges::copy(
      make_app_ext_datagram({.t = app::Ext::AddressOnly, .p = 2000u}),
      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }
  {
    State::Datagram datagram{.addr = random_loco_address()};
    std::ranges::copy(
      make_app_ext_datagram({.t = app::Ext::FillingStation, .p = 100u}),
      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }

  // app:dyn
  // Speed2
  {
    State::Datagram datagram{.addr = random_loco_address()};
    std::ranges::copy(make_app_dyn_datagram(random_interval<uint8_t>(), 1u),
                      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }
  // QoS
  {
    State::Datagram datagram{.addr = random_loco_address()};
    std::ranges::copy(
      make_app_dyn_datagram(random_interval<uint8_t>(0u, 100u), 7u),
      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }
  // Position (app:ext)
  {
    State::Datagram datagram{.addr = random_loco_address()};
    std::ranges::copy(make_app_dyn_datagram(random_interval<uint8_t>(), 20u),
                      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }
  {
    State::Datagram datagram{.addr = random_loco_address()};
    auto const first{make_app_dyn_datagram(random_interval<uint8_t>(), 20u)};
    std::ranges::copy(first, begin(datagram.bytes) + channel1_size);
    auto const second{
      make_app_dyn_datagram(random_interval<uint8_t>(0b001u, 0b111u), 20u)};
    std::ranges::copy(second,
                      begin(datagram.bytes) + channel1_size + size(first));
    state.datagrams.push_back(datagram);
  }
  // Status and alarm messages
  {
    State::Datagram datagram{.addr = random_loco_address()};
    std::ranges::copy(
      make_app_dyn_datagram(random_interval<uint8_t>(128u, 130u), 21u),
      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }
  {
    State::Datagram datagram{.addr = random_loco_address()};
    std::ranges::copy(make_app_dyn_datagram(0b1100'0110u, 21u),
                      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }
  // Temperature
  {
    State::Datagram datagram{.addr = random_loco_address()};
    std::ranges::copy(make_app_dyn_datagram(random_interval<uint8_t>(), 26u),
                      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }

  // app:xpom
  {
    State::Datagram datagram{.addr = random_loco_address()};
    std::array<uint8_t, 4uz> bytes{};
    std::ranges::for_each(
      bytes, [](uint8_t& byte) { byte = random_interval<uint8_t>(); });
    std::ranges::copy(make_app_xpom_datagram(0b00u, bytes),
                      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }
  {
    State::Datagram datagram{.addr = random_loco_address()};
    std::array<uint8_t, 4uz> bytes{};
    std::ranges::for_each(
      bytes, [](uint8_t& byte) { byte = random_interval<uint8_t>(); });
    std::ranges::copy(make_app_xpom_datagram(0b01u, bytes),
                      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }
  {
    State::Datagram datagram{.addr = random_loco_address()};
    std::array<uint8_t, 4uz> bytes{};
    std::ranges::for_each(
      bytes, [](uint8_t& byte) { byte = random_interval<uint8_t>(); });
    std::ranges::copy(make_app_xpom_datagram(0b10u, bytes),
                      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }
  {
    State::Datagram datagram{.addr = random_loco_address()};
    std::array<uint8_t, 4uz> bytes{};
    std::ranges::for_each(
      bytes, [](uint8_t& byte) { byte = random_interval<uint8_t>(); });
    std::ranges::copy(make_app_xpom_datagram(0b11u, bytes),
                      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }

  // app:CV-auto
  {
    State::Datagram datagram{.addr = random_loco_address()};
    std::ranges::copy(make_app_cv_auto_datagram(
                        random_interval<uint32_t>(0u, smath::pow(2u, 24u) - 1u),
                        random_interval<uint8_t>()),
                      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }

  // app:search
  {
    State::Datagram datagram{.addr = {.type = Address::Broadcast}};
    std::ranges::copy(
      make_app_search_datagram(
        random_basic_loco_address(), 0u, random_interval<uint8_t>(0u, 30u)),
      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }
  {
    State::Datagram datagram{.addr = {.type = Address::Broadcast}};
    std::ranges::copy(
      make_app_search_datagram(
        random_basic_loco_address(), 1u, random_interval<uint8_t>(0u, 30u)),
      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }
  {
    State::Datagram datagram{.addr = {.type = Address::Broadcast}};
    std::ranges::copy(
      make_app_search_datagram(
        random_extended_loco_address(), 0u, random_interval<uint8_t>(0u, 30u)),
      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }
}

// Add accessory datagrams
void add_accessory_datagrams() {
  // app:srq
  {
    State::Datagram datagram{.addr = random_basic_accessory_address()};
    std::ranges::copy(make_app_srq_datagram(datagram.addr),
                      begin(datagram.bytes));
    state.datagrams.push_back(datagram);
  }
  {
    State::Datagram datagram{.addr = random_extended_accessory_address()};
    std::ranges::copy(make_app_srq_datagram(datagram.addr),
                      begin(datagram.bytes));
    state.datagrams.push_back(datagram);
  }

  // app:stat4
  {
    State::Datagram datagram{.addr = random_basic_accessory_address()};
    std::ranges::copy(make_app_stat4_datagram(0b10011010u),
                      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }

  // app:stat1
  {
    State::Datagram datagram{.addr = random_basic_accessory_address()};
    std::ranges::copy(
      make_app_stat1_datagram(random_interval<uint8_t>(0u, 0b0111'1111u)),
      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }
  {
    State::Datagram datagram{.addr = random_extended_accessory_address()};
    std::ranges::copy(
      make_app_stat1_datagram(random_interval<uint8_t>(0u, 0b0111'1111u)),
      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }
  {
    auto const bit6{static_cast<bool>(random_interval(0, 1))};
    auto const bit5{static_cast<bool>(random_interval(0, 1))};
    auto const aspect{random_interval<uint8_t>(0b0011'1111u, 0b0111'1111u)};
    State::Datagram datagram{.addr = random_extended_accessory_address()};
    auto const first{
      make_app_stat1_datagram(bit6 << 6u | bit5 << 5u | (aspect & 0b1'1111u))};
    std::ranges::copy(first, begin(datagram.bytes) + channel1_size);
    auto const second{make_app_stat1_datagram(0x80u | aspect >> 5u)};
    std::ranges::copy(second,
                      begin(datagram.bytes) + channel1_size + size(first));
    state.datagrams.push_back(datagram);
  }

  // app:time
  {
    State::Datagram datagram{.addr = random_basic_accessory_address()};
    std::ranges::copy(
      make_app_time_datagram(static_cast<bool>(random_interval(0, 1)),
                             random_interval<uint8_t>(0u, 127u)),
      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }

  // app:error
  {
    State::Datagram datagram{.addr = random_basic_accessory_address()};
    std::ranges::copy(make_app_error_datagram(app::Error::InvalidCommand),
                      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }
  {
    State::Datagram datagram{.addr = random_basic_accessory_address()};
    std::ranges::copy(
      make_app_error_datagram(static_cast<app::Error::Code>(
        app::Error::AdditionalErrors | app::Error::ManualOperation)),
      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }

  // app:dyn
  // Status and alarm messages
  {
    State::Datagram datagram{.addr = random_basic_accessory_address()};
    std::ranges::copy(
      make_app_dyn_datagram(random_interval<uint8_t>(128u, 136u), 21u),
      begin(datagram.bytes) + channel1_size);
    state.datagrams.push_back(datagram);
  }
}

} // namespace

// Add demo packets and datagrams
void demo() {
  state.packets.clear();
  state.datagrams.clear();
  add_loco_packets();
  add_accessory_packets();
  add_service_packets();
  add_ack_nak_datagrams();
  add_loco_datagrams();
  add_accessory_datagrams();
}
