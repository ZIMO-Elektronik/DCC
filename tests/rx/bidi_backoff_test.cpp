#include "bidi_backoff_test.hpp"
#include <random>

BiDiBackoffTest::BiDiBackoffTest() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned int> dist(0, RAND_MAX);
  srand(dist(gen));
}

BiDiBackoffTest::~BiDiBackoffTest() {}

int BiDiBackoffTest::countTillFalse(int max) {
  int i{};
  for (; i < max; ++i)
    if (!_backoff) break;
  return i;
}