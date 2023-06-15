#include "bidi_logon_backoff_test.hpp"
#include <random>

BiDiLogonBackoffTest::BiDiLogonBackoffTest() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned int> dist(0, RAND_MAX);
  srand(dist(gen));
}

BiDiLogonBackoffTest::~BiDiLogonBackoffTest() {}

int BiDiLogonBackoffTest::countTillFalse(int max) {
  int i{};
  for (; i < max; ++i)
    if (!logon_backoff_) break;
  return i;
}