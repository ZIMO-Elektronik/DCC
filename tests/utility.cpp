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
