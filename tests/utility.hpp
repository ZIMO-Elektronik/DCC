#pragma once

#include <gtest/gtest.h>
#include <boost/preprocessor.hpp>
#include <concepts>
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
