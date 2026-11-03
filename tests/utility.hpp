#pragma once

#include <gtest/gtest.h>
#include <boost/preprocessor.hpp>
#include <chrono>
#include <concepts>
#include <dcc/dcc.hpp>
#include <random>

#define EXPECT_EQ_MACRO(r, first, elem) EXPECT_EQ(first, elem);

#define EXPECT_ALL_EQ(first, ...)                                              \
  do {                                                                         \
    BOOST_PP_SEQ_FOR_EACH(                                                     \
      EXPECT_EQ_MACRO, first, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))           \
  } while (0)

#define EXPECT_ALL_TRUE(...)                                                   \
  do {                                                                         \
    BOOST_PP_SEQ_FOR_EACH(                                                     \
      EXPECT_EQ_MACRO, true, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))            \
  } while (0)

void add_system_clock_offset(std::chrono::nanoseconds offset);
void reset_system_clock_offset();

template<typename Rep, typename Period>
void add_system_clock_offset(std::chrono::duration<Rep, Period> offset) {
  add_system_clock_offset(
    std::chrono::duration_cast<std::chrono::nanoseconds>(offset));
}

template<typename T>
requires(std::integral<T> || std::floating_point<T>)
constexpr T random_interval(T min = std::numeric_limits<T>::min(),
                            T max = std::numeric_limits<T>::max()) {
  std::mt19937 gen{std::random_device{}()};
  if constexpr (std::integral<T>) {
    std::uniform_int_distribution<T> dis{min, max};
    return dis(gen);
  } else {
    std::uniform_real_distribution<T> dis{min, max};
    return dis(gen);
  }
}

std::vector<dcc::bidi::Datagram<dcc::bidi::datagram_size<dcc::bidi::Bits::_48>>>
make_app_get_data_datagrams(std::span<uint8_t const> cvs, uint8_t data_space);
