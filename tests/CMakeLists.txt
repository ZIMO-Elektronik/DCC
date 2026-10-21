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
  1.92.0
  URL
  https://github.com/boostorg/boost/releases/download/boost-1.92.0/boost-1.92.0-cmake.tar.xz
  URL_HASH
  SHA256=9bed76128d4e46755dbe818487788c6fceb6f72b378f4daa49b7e1e600d9088d
  OPTIONS
  "BOOST_ENABLE_CMAKE ON"
  "BOOST_SKIP_INSTALL_RULES ON"
  "BUILD_SHARED_LIBS OFF"
  "BOOST_INCLUDE_LIBRARIES preprocessor")
cpmaddpackage(URI "gh:google/googletest#main" OPTIONS "INSTALL_GTEST OFF")

target_link_libraries(DCCTests PRIVATE Boost::preprocessor DCC::DCC
                                       GTest::gtest_main GTest::gmock)

gtest_discover_tests(DCCTests)
