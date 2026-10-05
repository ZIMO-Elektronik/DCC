#pragma once

#include <gtest/gtest.h>
#include <boost/preprocessor.hpp>
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

// Make vector of get_data datagrams from CVs and data space index
constexpr auto make_get_data_datagrams(std::span<uint8_t const> cvs,
                                       uint8_t data_space) {
  std::vector<uint8_t> bytes;
  uint8_t crc;

  // Bytes
  for (auto i{0uz}; i < size(cvs); ++i) {
    // Header: 0, 33, 66, 99, ...
    if (!(i % 31uz)) {
      auto const block_size{std::min<size_t>(31uz, size(cvs) - i)};
      bytes.emplace_back(
        static_cast<uint8_t>((i ? ztl::mask<5u> : 0u) | block_size));
      crc = dcc::crc8(bytes.back() ^ data_space);
    }

    // Data
    bytes.emplace_back(cvs[i]);
    crc = dcc::crc8(bytes.back() ^ crc);

    // CRC: 32, 65, 98, 131, ...
    if (i % 31uz == std::min<size_t>(31uz, size(cvs) - (i / 31uz) * 31uz) - 1uz)
      bytes.emplace_back(crc);
  }

  // Special continuation block (header only)
  if (!empty(cvs) && !(size(cvs) % 31uz)) bytes.emplace_back(ztl::mask<5u>);

  // Resize to multiple of 6 (cutout size)
  bytes.resize((size(bytes) + 5uz) / 6uz * 6uz, 0u);

  std::vector<
    dcc::bidi::Datagram<dcc::bidi::datagram_size<dcc::bidi::Bits::_48>>>
    retval;
  for (auto chunk : bytes | std::views::chunk(6))
    retval.push_back(dcc::bidi::make_get_data_datagram(
      std::span<uint8_t, 6uz>{cbegin(chunk), cend(chunk)}));

  return retval;
}
