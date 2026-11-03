#include "utility.hpp"
#include <sys/syscall.h>
#include <unistd.h>
#include <atomic>
#include <chrono>
#include <ctime>

namespace {

std::atomic<std::chrono::nanoseconds::rep> system_clock_offset{};

} // namespace

// Overwrite clock_gettime to apply offset to system clock
extern "C" int clock_gettime(clockid_t id, timespec* tp) {
  // Get actual timespec from syscall
  if (auto const err{syscall(SYS_clock_gettime, id, tp)}; err != 0)
    return static_cast<int>(err);

  // Apply offset
  static constexpr auto nanoseconds_per_second{1'000'000'000L};
  auto const offset{system_clock_offset.load(std::memory_order_relaxed)};
  auto const seconds{offset / nanoseconds_per_second};
  auto const nanoseconds{offset % nanoseconds_per_second};
  tp->tv_sec += seconds;
  tp->tv_nsec += nanoseconds;
  if (tp->tv_nsec >= nanoseconds_per_second) {
    ++tp->tv_sec;
    tp->tv_nsec -= nanoseconds_per_second;
  } else if (tp->tv_nsec < 0) {
    --tp->tv_sec;
    tp->tv_nsec += nanoseconds_per_second;
  }

  return 0;
}

// Increase (or decrease) system clock offset
void add_system_clock_offset(std::chrono::nanoseconds offset) {
  system_clock_offset.fetch_add(offset.count(), std::memory_order_relaxed);
}

// Reset system clock offset
void reset_system_clock_offset() { system_clock_offset.store(0); }

// Make vector of get_data datagrams from CVs and data space index
std::vector<dcc::bidi::Datagram<dcc::bidi::datagram_size<dcc::bidi::Bits::_48>>>
make_app_get_data_datagrams(std::span<uint8_t const> cvs, uint8_t data_space) {
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

  // Header & CRC only for special starting block
  if (empty(cvs)) {
    bytes.emplace_back(0u);
    bytes.emplace_back(dcc::crc8(0u ^ data_space));
  }
  // ... or continuation block
  else if (!(size(cvs) % 31uz)) {
    bytes.emplace_back(ztl::mask<5u>);
    bytes.emplace_back(dcc::crc8(ztl::mask<5u> ^ data_space));
  }

  // Resize to multiple of 6 (cutout size)
  bytes.resize((size(bytes) + 5uz) / 6uz * 6uz, 0u);

  std::vector<
    dcc::bidi::Datagram<dcc::bidi::datagram_size<dcc::bidi::Bits::_48>>>
    retval;
  for (auto chunk : bytes | std::views::chunk(6))
    retval.push_back(dcc::bidi::make_app_get_data_datagram(
      std::span<uint8_t, 6uz>{cbegin(chunk), cend(chunk)}));

  return retval;
}
