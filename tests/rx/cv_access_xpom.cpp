#include "rx_test.hpp"

TEST_F(RxTest, cv_access_xpom_verify_bytes) {
  auto ss{RandomInterval<uint8_t>(0u, 3u)};
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto packet{make_cv_access_xpom_verify_packet(_addrs.primary, ss, cv_addr)};

  InSequence s;
  EXPECT_CALL(_mock, readCv(cv_addr + 0u)).WillOnce(Return(0u));
  EXPECT_CALL(_mock, readCv(cv_addr + 1u)).WillOnce(Return(1u));
  EXPECT_CALL(_mock, readCv(cv_addr + 2u)).WillOnce(Return(2u));
  EXPECT_CALL(_mock, readCv(cv_addr + 3u)).WillOnce(Return(3u));

  ReceiveAndExecute(packet);
}

TEST_F(RxTest, cv_access_xpom_write_bytes) {
  {
    uint8_t ss{0b00u};
    auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
    auto cv0{RandomInterval<uint8_t>(0u, 255u)};
    auto packet{
      make_cv_access_xpom_write_packet(_addrs.primary, ss, cv_addr, cv0)};

    InSequence s;
    EXPECT_CALL(_mock, writeCv(cv_addr + 0u, cv0)).WillOnce(Return(0u));
    EXPECT_CALL(_mock, readCv(cv_addr + 1u)).WillOnce(Return(1u));
    EXPECT_CALL(_mock, readCv(cv_addr + 2u)).WillOnce(Return(2u));
    EXPECT_CALL(_mock, readCv(cv_addr + 3u)).WillOnce(Return(3u));

    ReceiveAndExecuteTwice(packet);
  }

  {
    uint8_t ss{0b01u};
    auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
    auto cv0{RandomInterval<uint8_t>(0u, 255u)};
    auto cv1{RandomInterval<uint8_t>(0u, 255u)};
    auto packet{
      make_cv_access_xpom_write_packet(_addrs.primary, ss, cv_addr, cv0, cv1)};

    InSequence s;
    EXPECT_CALL(_mock, writeCv(cv_addr + 0u, cv0)).WillOnce(Return(0u));
    EXPECT_CALL(_mock, writeCv(cv_addr + 1u, cv1)).WillOnce(Return(1u));
    EXPECT_CALL(_mock, readCv(cv_addr + 2u)).WillOnce(Return(2u));
    EXPECT_CALL(_mock, readCv(cv_addr + 3u)).WillOnce(Return(3u));

    ReceiveAndExecuteTwice(packet);
  }

  {
    uint8_t ss{0b10u};
    auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
    auto cv0{RandomInterval<uint8_t>(0u, 255u)};
    auto cv1{RandomInterval<uint8_t>(0u, 255u)};
    auto cv2{RandomInterval<uint8_t>(0u, 255u)};
    auto packet{make_cv_access_xpom_write_packet(
      _addrs.primary, ss, cv_addr, cv0, cv1, cv2)};

    InSequence s;
    EXPECT_CALL(_mock, writeCv(cv_addr + 0u, cv0)).WillOnce(Return(cv0));
    EXPECT_CALL(_mock, writeCv(cv_addr + 1u, cv1)).WillOnce(Return(cv1));
    EXPECT_CALL(_mock, writeCv(cv_addr + 2u, cv2)).WillOnce(Return(cv2));
    EXPECT_CALL(_mock, readCv(cv_addr + 3u)).WillOnce(Return(3u));

    ReceiveAndExecuteTwice(packet);
  }

  {
    uint8_t ss{0b11u};
    auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
    auto cv0{RandomInterval<uint8_t>(0u, 255u)};
    auto cv1{RandomInterval<uint8_t>(0u, 255u)};
    auto cv2{RandomInterval<uint8_t>(0u, 255u)};
    auto cv3{RandomInterval<uint8_t>(0u, 255u)};
    auto packet{make_cv_access_xpom_write_packet(
      _addrs.primary, ss, cv_addr, cv0, cv1, cv2, cv3)};

    InSequence s;
    EXPECT_CALL(_mock, writeCv(cv_addr + 0u, cv0)).WillOnce(Return(cv0));
    EXPECT_CALL(_mock, writeCv(cv_addr + 1u, cv1)).WillOnce(Return(cv1));
    EXPECT_CALL(_mock, writeCv(cv_addr + 2u, cv2)).WillOnce(Return(cv2));
    EXPECT_CALL(_mock, writeCv(cv_addr + 3u, cv3)).WillOnce(Return(cv3));

    ReceiveAndExecuteTwice(packet);
  }
}

TEST_F(RxTest, cv_access_xpom_write_bit) {
  auto ss{RandomInterval<uint8_t>(0u, 3u)};
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto bit{static_cast<bool>(RandomInterval(0u, 1u))};
  auto pos{RandomInterval(0u, 7u)};
  auto packet{
    make_cv_access_xpom_write_packet(_addrs.primary, ss, cv_addr, bit, pos)};

  InSequence s;
  EXPECT_CALL(_mock, writeCv(cv_addr + 0u, bit, pos));
  EXPECT_CALL(_mock, readCv(cv_addr + 0u)).WillOnce(Return(0u));
  EXPECT_CALL(_mock, readCv(cv_addr + 1u)).WillOnce(Return(1u));
  EXPECT_CALL(_mock, readCv(cv_addr + 2u)).WillOnce(Return(2u));
  EXPECT_CALL(_mock, readCv(cv_addr + 3u)).WillOnce(Return(3u));

  ReceiveAndExecuteTwice(packet);
}
