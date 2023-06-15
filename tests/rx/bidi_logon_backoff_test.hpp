#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <dcc/dcc.hpp>

using dcc::rx::bidi::LogonBackoff;

struct BiDiLogonBackoffTest : ::testing::Test {
  BiDiLogonBackoffTest();
  virtual ~BiDiLogonBackoffTest();

  int countTillFalse(int max);

  LogonBackoff logon_backoff_{};
};
