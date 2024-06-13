#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <dcc/dcc.hpp>

struct BackoffTest : ::testing::Test {
protected:
  BackoffTest();
  virtual ~BackoffTest();

  int CountTillFalse(int max);

  dcc::rx::Backoff _backoff{};
};
