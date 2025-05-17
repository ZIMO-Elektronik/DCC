#include "backoff_test.hpp"
#include <random>

BackoffTest::BackoffTest() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned int> dist(0, RAND_MAX);
  srand(dist(gen));
}

BackoffTest::~BackoffTest() {}

int BackoffTest::CountTillFalse(int max) {
  int i{};
  for (; i < max; ++i)
    if (!_backoff) break;
  return i;
}
