#include "rx_test.hpp"

TEST_F(RxTest, cv_long_verify_bit_service_mode) {
  EnterServiceMode();

  // Don't write any CV which might trigger config (e.g. 1, 28, ...)!
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto bit{RandomInterval(0u, 1u)};
  auto position{RandomInterval(0u, 7u)};
  auto packet{
    dcc::make_cv_access_long_verify_service_packet(cv_addr, bit, position)};

  // 5 or more identical packets
  EXPECT_CALL(_mock, readCv(cv_addr, bit, position)).WillOnce(Return(bit));
  EXPECT_CALL(_mock, serviceAck());
  for (auto i{0uz}; i < 5uz; ++i) {
    Receive(packet);
    Execute();
  }
}

TEST_F(RxTest, cv_long_verify_byte_operations_mode) {
  auto cv_addr{RandomInterval(0u, smath::pow(2u, 10u) - 1u)};
  auto packet{dcc::make_cv_access_long_verify_packet(_addrs.primary, cv_addr)};

  EXPECT_CALL(_mock, readCv(cv_addr, 0u));
  Receive(packet);
  Execute();
}

TEST_F(RxTest, cv_long_verify_byte_service_mode) {
  EnterServiceMode();

  auto cv_addr{RandomInterval(0u, smath::pow(2u, 10u) - 1u)};
  auto packet{dcc::make_cv_access_long_verify_service_packet(cv_addr, 42u)};

  // 5 or more identical packets
  EXPECT_CALL(_mock, readCv(cv_addr, 42u)).WillOnce(Return(42u));
  EXPECT_CALL(_mock, serviceAck());
  for (auto i{0uz}; i < 5uz; ++i) {
    Receive(packet);
    Execute();
  }
}

TEST_F(RxTest, cv_long_ignore_write_bit_operations_mode) {
  // Don't write any CV which might trigger config (e.g. 1, 28, ...)!
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto bit{RandomInterval(0u, 1u)};
  auto position{RandomInterval(0u, 7u)};
  auto packet{dcc::make_cv_access_long_write_packet(
    _addrs.primary, cv_addr, bit, position)};

  // 2 or more identical packets
  EXPECT_CALL(_mock, writeCv(cv_addr, bit, position)).Times(0);
  for (auto i{0uz}; i < 2uz; ++i) {
    Receive(packet);
    Execute();
  }
}

TEST_F(RxTest, cv_long_write_bit_service_mode) {
  EnterServiceMode();

  // Don't write any CV which might trigger config (e.g. 1, 28, ...)!
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto bit{RandomInterval(0u, 1u)};
  auto position{RandomInterval(0u, 7u)};
  auto packet{
    dcc::make_cv_access_long_write_service_packet(cv_addr, bit, position)};

  // 5 or more identical packets
  EXPECT_CALL(_mock, writeCv(cv_addr, bit, position)).WillOnce(Return(bit));
  EXPECT_CALL(_mock, serviceAck());
  for (auto i{0uz}; i < 5uz; ++i) {
    Receive(packet);
    Execute();
  }
}

TEST_F(RxTest, cv_long_write_byte_operations_mode) {
  // Don't write any CV which might trigger config (e.g. 1, 28, ...)!
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto byte{RandomInterval<uint8_t>(0u, 255u)};

  EXPECT_CALL(_mock, writeCv(cv_addr, byte));
  ReceiveAndExecuteTwoIdenticalCvLongWritePackets(
    _addrs.primary, cv_addr, byte);
}

TEST_F(
  RxTest,
  cv_long_write_byte_operations_mode_requires_two_identical_not_back_to_back_packets) {
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto byte{RandomInterval<uint8_t>(0u, 255u)};
  auto cv_packet{
    dcc::make_cv_access_long_write_packet(_addrs.primary, cv_addr, byte)};

  EXPECT_CALL(_mock, writeCv(cv_addr, byte));

  Receive(cv_packet);
  Execute();

  auto other_packet_to_different_address{
    dcc::make_function_group_f4_f0_packet(42u, 0b1u)};
  Receive(other_packet_to_different_address);
  Execute();

  Receive(cv_packet);
  Execute();
}

TEST_F(
  RxTest,
  cv_long_write_byte_operations_mode_interrupted_by_packet_to_same_address) {
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto byte{RandomInterval<uint8_t>(0u, 255u)};
  auto cv_packet{
    dcc::make_cv_access_long_write_packet(_addrs.primary, cv_addr, byte)};

  EXPECT_CALL(_mock, writeCv(cv_addr, byte)).Times(0);

  Receive(cv_packet);
  Execute();

  auto other_packet_to_same_address{
    dcc::make_function_group_f4_f0_packet(_addrs.primary, 0b1u)};
  Receive(other_packet_to_same_address);
  Execute();

  Receive(cv_packet);
  Execute();
}

TEST_F(RxTest, cv_long_write_byte_service_mode) {
  EnterServiceMode();

  // Don't write any CV which might trigger config (e.g. 1, 28, ...)!
  auto cv_addr{RandomInterval(30u, smath::pow(2u, 10u) - 1u)};
  auto byte{RandomInterval<uint8_t>(0u, 255u)};
  auto packet{dcc::make_cv_access_long_write_service_packet(cv_addr, byte)};

  // 5 or more identical packets
  EXPECT_CALL(_mock, writeCv(cv_addr, byte));
  for (auto i{0uz}; i < 5uz; ++i) {
    Receive(packet);
    Execute();
  }
}
