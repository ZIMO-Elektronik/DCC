#include <gtest/gtest.h>
#include <dcc/dcc.hpp>

static_assert(std::is_trivially_copyable_v<dcc::bidi::Dissector>);
static_assert(std::input_iterator<dcc::bidi::Dissector>);
static_assert(std::ranges::range<dcc::bidi::Dissector>);
static_assert(std::ranges::input_range<dcc::bidi::Dissector>);

using namespace dcc::bidi;

TEST(Dissector, channel_1_and_2) {
  {
    dcc::Packet packet{0x03u, 0x3Fu, 0x80u, 0xBCu};
    Datagram<> datagram{0xA3u, 0xACu, 0x59u, 0xB1u, 0x66, 0x5Au, 0xACu, 0xACu};
    Dissector dissector{datagram, packet};
    std::vector<Dissector::value_type> expected{
      app::AdrHigh{.d = 0u},        // Address
      app::Dyn{.d = 79u, .x = 26u}, // Temperature
      app::Dyn{.d = 0u, .x = 0u}};  // Speed
    std::vector<Dissector::value_type> result{};
    std::ranges::copy(dissector, back_inserter(result));
    EXPECT_EQ(expected, result);
  }

  {
    dcc::Packet packet{0x03u, 0xDEu, 0x00u, 0xDDu};
    Datagram<> datagram{0xA3u, 0xACu, 0x5Au, 0xACu, 0x9Au, 0x5Au, 0xACu, 0xACu};
    Dissector dissector{datagram, packet};
    std::vector<Dissector::value_type> expected{
      app::AdrHigh{.d = 0u},       // Address
      app::Dyn{.d = 0u, .x = 7u},  // Speed
      app::Dyn{.d = 0u, .x = 0u}}; // QoS
    std::vector<Dissector::value_type> result{};
    std::ranges::copy(dissector, back_inserter(result));
    EXPECT_EQ(expected, result);
  }

  {
    dcc::Packet packet{0x03u, 0xA0u, 0xA3u};
    Datagram<> datagram{0x99u, 0xA5u, 0x59u, 0x2Eu, 0xD2u, 0x00u, 0x00u, 0x00u};
    Dissector dissector{datagram, packet};
    std::vector<Dissector::value_type> expected{
      app::AdrLow{.d = 3u},           // Address
      app::Dyn{.d = 119u, .x = 46u}}; // Track voltage
    std::vector<Dissector::value_type> result{};
    std::ranges::copy(dissector, back_inserter(result));
    EXPECT_EQ(expected, result);
  }

  {
    dcc::Packet packet{0x03u, 0xDFu, 0x00u, 0xDCu};
    Datagram<> datagram{0xA3u, 0xACu, 0x5Au, 0x74u, 0x5Cu, 0x5Au, 0xACu, 0xACu};
    Dissector dissector{datagram, packet};
    std::vector<Dissector::value_type> expected{
      app::AdrHigh{.d = 0u},        // Address
      app::Dyn{.d = 19u, .x = 27u}, // Direction status byte
      app::Dyn{.d = 0u, .x = 0u}};  // QoS
    std::vector<Dissector::value_type> result{};
    std::ranges::copy(dissector, back_inserter(result));
    EXPECT_EQ(expected, result);
  }

  {
    dcc::Packet packet{0xC5u, 0x39u, 0xB0u, 0x4Cu};
    Datagram<> datagram{0x9Cu, 0xA6u, 0x5Au, 0xA3u, 0xACu, 0x00u, 0x00u, 0x00u};
    Dissector dissector{datagram, packet};
    std::vector<Dissector::value_type> expected{
      app::AdrHigh{.d = 133u},     // Address
      app::Dyn{.d = 4u, .x = 0u}}; // Speed
    std::vector<Dissector::value_type> result{};
    std::ranges::copy(dissector, back_inserter(result));
    EXPECT_EQ(expected, result);
  }

  {
    dcc::Packet packet{0xC5u, 0x39u, 0xE4u, 0x00u, 0x00u, 0x18u};
    Datagram<> datagram{0x99u, 0x3Au, 0xACu, 0xA5u, 0x00u, 0x00u, 0x00u, 0x00u};
    Dissector dissector{datagram, packet};
    std::vector<Dissector::value_type> expected{
      app::AdrLow{.d = 57u}, // Address
      app::Pom{.d = 3u}};    // Pom
    std::vector<Dissector::value_type> result{};
    std::ranges::copy(dissector, back_inserter(result));
    EXPECT_EQ(expected, result);
  }

  {
    dcc::Packet packet{0x03u, 0x3Fu, 0x80u, 0xBCu};
    Datagram<> datagram{0x99u, 0xA5u, 0x0Fu, 0x0Fu, 0x00u, 0x00u, 0x00u, 0x00u};
    Dissector dissector{datagram, packet};
    std::vector<Dissector::value_type> expected{app::AdrLow{.d = 3u}, // Address
                                                acks[0uz]};           // ACK
    std::vector<Dissector::value_type> result{};
    std::ranges::copy(dissector, back_inserter(result));
    EXPECT_EQ(expected, result);
  }

  {
    dcc::Packet packet{0x03u, 0x3Fu, 0xB6u, 0x8Au};
    Datagram<> datagram{0xA3u, 0xACu, 0xACu, 0xA5u, 0x0Fu, 0x0Fu, 0x00u, 0x00u};
    Dissector dissector{datagram, packet};
    std::vector<Dissector::value_type> expected{
      app::AdrHigh{.d = 0u}, // Address
      app::Pom{.d = 3u},     // Pom
      acks[0uz]};            // ACK
    std::vector<Dissector::value_type> result{};
    std::ranges::copy(dissector, back_inserter(result));
    EXPECT_EQ(expected, result);
  }
}

TEST(Dissector, channel_1) {
  {
    dcc::Packet packet{0x03u, 0xA0u, 0xA3u};
    Datagram<> datagram{0x99u, 0xA5u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u};
    Dissector dissector{datagram, packet};
    std::vector<Dissector::value_type> expected{
      app::AdrLow{.d = 3u}}; // Address
    std::vector<Dissector::value_type> result{};
    std::ranges::copy(dissector, back_inserter(result));
    EXPECT_EQ(expected, result);
  }
}

TEST(Dissector, channel_2) {
  {
    dcc::Packet packet{0x03u, 0x3Fu, 0x80u, 0xBCu};
    Datagram<> datagram{0x00u, 0x00u, 0x59u, 0xB1u, 0x66, 0x5Au, 0xACu, 0xACu};
    Dissector dissector{datagram, packet};
    std::vector<Dissector::value_type> expected{
      app::Dyn{.d = 79u, .x = 26u}, // Temperature
      app::Dyn{.d = 0u, .x = 0u}};  // Speed
    std::vector<Dissector::value_type> result{};
    std::ranges::copy(dissector, back_inserter(result));
    EXPECT_EQ(expected, result);
  }
}

TEST(Dissector, invalid) {
  {
    dcc::Packet packet{0x03u, 0x3Fu, 0x80u, 0xBCu};
    Datagram<> datagram{0x00u, 0x00u, 0xD9u, 0xB1u, 0x66, 0x5Au, 0xACu, 0xACu};
    Dissector dissector{datagram, packet};
    std::vector<Dissector::value_type> expected{};
    std::vector<Dissector::value_type> result{};
    std::ranges::copy(dissector, back_inserter(result));
    EXPECT_EQ(expected, result);
  }
}
