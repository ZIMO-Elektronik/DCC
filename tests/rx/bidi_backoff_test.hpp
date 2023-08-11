#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <dcc/dcc.hpp>

using dcc::rx::bidi::Backoff;

struct BiDiBackoffTest : ::testing::Test {
  BiDiBackoffTest();
  virtual ~BiDiBackoffTest();

  int countTillFalse(int max);

  Backoff _backoff{};
};
