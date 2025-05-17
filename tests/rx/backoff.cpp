#include "backoff_test.hpp"

TEST_F(BackoffTest, initial_backoff_is_always_false) {
  EXPECT_FALSE(static_cast<bool>(_backoff));
}

TEST_F(BackoffTest, repeated_values_stay_below_max_range) {
  EXPECT_LT(CountTillFalse(CHAR_BIT), CHAR_BIT);
  EXPECT_LT(CountTillFalse(CHAR_BIT << 1), CHAR_BIT << 1);
  EXPECT_LT(CountTillFalse(CHAR_BIT << 2), CHAR_BIT << 2);
  EXPECT_LT(CountTillFalse(CHAR_BIT << 3), CHAR_BIT << 3);

  // Range can't get bigger than CHAR_BIT << 3 (64)
  EXPECT_LT(CountTillFalse(CHAR_BIT << 4), CHAR_BIT << 3);

  // Range can be reset
  _backoff = {};
  EXPECT_LT(CountTillFalse(CHAR_BIT), CHAR_BIT);
}

TEST_F(BackoffTest, now) {
  EXPECT_LT(CountTillFalse(CHAR_BIT), CHAR_BIT);
  EXPECT_LT(CountTillFalse(CHAR_BIT << 1), CHAR_BIT << 1);
  EXPECT_LT(CountTillFalse(CHAR_BIT << 2), CHAR_BIT << 2);
  EXPECT_LT(CountTillFalse(CHAR_BIT << 3), CHAR_BIT << 3);

  // Range can't get bigger than CHAR_BIT << 3 (64)
  EXPECT_LT(CountTillFalse(CHAR_BIT << 4), CHAR_BIT << 3);

  // Range can be reset with now
  _backoff.now();
  EXPECT_FALSE(static_cast<bool>(_backoff));
  EXPECT_LT(CountTillFalse(CHAR_BIT), CHAR_BIT);
}
