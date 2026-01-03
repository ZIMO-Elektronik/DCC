file(GLOB_RECURSE SRC src/*.cpp)
add_executable(DCCRepl ${SRC})

target_common_warnings(DCCRepl PRIVATE)
target_common_errors(DCCRepl PRIVATE)

if(NOT TARGET cli::cli)
  cpmaddpackage("gh:daniele77/cli@2.2.0")
endif()

target_link_libraries(DCCRepl PRIVATE cli::cli DCC::DCC)
