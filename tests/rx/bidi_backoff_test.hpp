#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <dcc/dcc.hpp>

using dcc::rx::bidi::Backoff;

struct BiDiBackoffTest : ::testing::Test {
protected:
  BiDiBackoffTest();
  virtual ~BiDiBackoffTest();

  int CountTillFalse(int max);

  Backoff _backoff{};
};
