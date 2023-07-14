file(GLOB_RECURSE SRC *.cpp)
add_executable(DCCReplExample ${SRC})

target_common_warnings(DCCReplExample PRIVATE)

cpmaddpackage("gh:daniele77/cli@2.0.2")

target_link_libraries(DCCReplExample PRIVATE cli::cli DCC::DCC)
