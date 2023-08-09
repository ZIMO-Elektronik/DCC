#include "bidi_backoff_test.hpp"

TEST_F(BiDiBackoffTest, repeated_values_stay_below_max_range) {
  EXPECT_LT(countTillFalse(CHAR_BIT), CHAR_BIT);
  EXPECT_LT(countTillFalse(CHAR_BIT << 1), CHAR_BIT << 1);
  EXPECT_LT(countTillFalse(CHAR_BIT << 2), CHAR_BIT << 2);
  EXPECT_LT(countTillFalse(CHAR_BIT << 3), CHAR_BIT << 3);

  // Range can't get bigger than CHAR_BIT << 3 (64)
  EXPECT_LT(countTillFalse(CHAR_BIT << 4), CHAR_BIT << 3);

  // Range can be reset
  _backoff = {};
  EXPECT_LT(countTillFalse(CHAR_BIT), CHAR_BIT);
}

TEST_F(BiDiBackoffTest, reset) {
  EXPECT_LT(countTillFalse(CHAR_BIT), CHAR_BIT);
  EXPECT_LT(countTillFalse(CHAR_BIT << 1), CHAR_BIT << 1);
  EXPECT_LT(countTillFalse(CHAR_BIT << 2), CHAR_BIT << 2);
  EXPECT_LT(countTillFalse(CHAR_BIT << 3), CHAR_BIT << 3);

  // Range can't get bigger than CHAR_BIT << 3 (64)
  EXPECT_LT(countTillFalse(CHAR_BIT << 4), CHAR_BIT << 3);

  // Range can be reset
  _backoff = {};
  EXPECT_LT(countTillFalse(CHAR_BIT), CHAR_BIT);
}