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
  1.88.0
  URL
  https://github.com/boostorg/boost/releases/download/boost-1.88.0/boost-1.88.0-cmake.tar.gz
  URL_HASH
  SHA256=dcea50f40ba1ecfc448fdf886c0165cf3e525fef2c9e3e080b9804e8117b9694
  OPTIONS
  "BOOST_ENABLE_CMAKE ON"
  "BOOST_SKIP_INSTALL_RULES ON"
  "BUILD_SHARED_LIBS OFF"
  "BOOST_INCLUDE_LIBRARIES preprocessor")
cpmaddpackage(URI "gh:google/googletest#main" OPTIONS "INSTALL_GTEST OFF")

target_link_libraries(DCCTests PRIVATE Boost::preprocessor DCC::DCC
                                       GTest::gtest_main GTest::gmock)

gtest_discover_tests(DCCTests)
