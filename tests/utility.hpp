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

template<std::unsigned_integral T>
static T RandomInterval(T min, T max) {
  std::mt19937 gen{std::random_device{}()};
  std::uniform_int_distribution<T> dis{min, max};
  return dis(gen);
}
