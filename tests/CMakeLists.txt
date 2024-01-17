include(GoogleTest)

file(GLOB_RECURSE SRC *.cpp)
add_executable(DCCTests ${SRC})

sanitize(address,undefined)

target_common_warnings(DCCTests PRIVATE)

cpmaddpackage("gh:google/googletest#main")

target_link_libraries(DCCTests PRIVATE DCC::DCC GTest::gtest GTest::gmock)

gtest_discover_tests(DCCTests)
