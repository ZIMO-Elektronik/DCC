#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <dcc/dcc.hpp>
#include <vector>

TEST(crc8, crc8) {
  std::vector<uint8_t> data{0x0Bu,
                            0x0Au,
                            0x00u,
                            0x00u,
                            0x8Eu,
                            0x40u,
                            0x00u,
                            0x0Du,
                            0x67u,
                            0x00u,
                            0x01u,
                            0x00u};
  EXPECT_EQ(dcc::crc8(data), 0x4Cu);

  dcc::Packet packet{0x0Bu,
                     0x0Au,
                     0x00u,
                     0x00u,
                     0x8Eu,
                     0x40u,
                     0x00u,
                     0x0Du,
                     0x67u,
                     0x00u,
                     0x01u,
                     0x00u,
                     0xFFu};  // Packet checksum is not part of CRC
  EXPECT_EQ(dcc::crc8(packet), 0x4Cu);

  uint8_t crc{};
  for (auto b : data) crc = dcc::crc8(b ^ crc);
  EXPECT_EQ(crc, 0x4Cu);
}