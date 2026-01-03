include(GoogleTest)

file(GLOB_RECURSE SRC *.cpp)
add_executable(DCCTests ${SRC})

sanitize(address,undefined)

target_common_warnings(DCCTests PRIVATE)
target_common_errors(DCCTests PRIVATE -Werror)

cpmaddpackage(
  NAME
  Boost
  VERSION
  1.90.0
  URL
  https://github.com/boostorg/boost/releases/download/boost-1.90.0/boost-1.90.0-cmake.tar.gz
  URL_HASH
  SHA256=913ca43d49e93d1b158c9862009add1518a4c665e7853b349a6492d158b036d4
  OPTIONS
  "BOOST_ENABLE_CMAKE ON"
  "BOOST_SKIP_INSTALL_RULES ON"
  "BUILD_SHARED_LIBS OFF"
  "BOOST_INCLUDE_LIBRARIES preprocessor")
cpmaddpackage(URI "gh:google/googletest#main" OPTIONS "INSTALL_GTEST OFF")

target_link_libraries(DCCTests PRIVATE Boost::preprocessor DCC::DCC
                                       GTest::gtest_main GTest::gmock)

gtest_discover_tests(DCCTests)
