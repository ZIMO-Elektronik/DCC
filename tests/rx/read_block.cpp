#include "rx_test.hpp"

using namespace dcc::bidi;

TEST_F(RxTest, read_data_spaces) { ReadDataSpace(0u); }

TEST_F(RxTest, read_data_space_1) { ReadDataSpace(1u); }

TEST_F(RxTest, read_data_space_2) { ReadDataSpace(2u); }

TEST_F(RxTest, read_data_space_4) { ReadDataSpace(4u); }

TEST_F(RxTest, read_data_space_5) { ReadDataSpace(5u); }

TEST_F(RxTest, read_data_space_6) { ReadDataSpace(6u); }

TEST_F(RxTest, read_data_space_7) { ReadDataSpace(7u); }
