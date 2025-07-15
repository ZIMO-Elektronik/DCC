#pragma once

#include <gtest/gtest.h>
#include <boost/preprocessor.hpp>

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
