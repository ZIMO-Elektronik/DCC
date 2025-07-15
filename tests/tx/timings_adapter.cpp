#include <gtest/gtest.h>
#include <dcc/dcc.hpp>

static_assert(std::is_trivially_copyable_v<dcc::tx::TimingsAdapter>);
static_assert(std::ranges::range<dcc::tx::TimingsAdapter>);
static_assert(std::ranges::input_range<dcc::tx::TimingsAdapter>);
static_assert(std::input_iterator<dcc::tx::TimingsAdapter::iterator>);

TEST(TimingsAdapter, compare_to_packet2timings) {
  {
    auto packet{dcc::make_idle_packet()};
    auto timings{dcc::tx::packet2timings(packet)};
    dcc::tx::TimingsAdapter timings_range{packet, dcc::tx::Config{}};
    EXPECT_TRUE(std::ranges::equal(timings, timings_range));
  }

  {
    auto packet{
      dcc::make_advanced_operations_speed_direction_and_functions_packet(
        42u, 100u, 10u, 19u)};
    auto timings{dcc::tx::packet2timings(packet)};
    dcc::tx::TimingsAdapter timings_range{packet, dcc::tx::Config{}};
    EXPECT_TRUE(std::ranges::equal(timings, timings_range));
  }
}
