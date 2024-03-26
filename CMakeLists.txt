cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
include(FetchContent)

if(ESP_PLATFORM)
  file(GLOB_RECURSE SRC src/*.c)
  idf_component_register(SRCS ${SRC} INCLUDE_DIRS include REQUIRES driver)
  set_target_properties(${COMPONENT_LIB} PROPERTIES PREFIX "")
  set_target_properties(
    ${COMPONENT_LIB} PROPERTIES OUTPUT_NAME
                                ${CMAKE_STATIC_LIBRARY_PREFIX}${COMPONENT_LIB})
  target_link_libraries(${COMPONENT_LIB} PUBLIC DCC)
endif()

FetchContent_Declare(
  CMakeModules
  GIT_REPOSITORY https://github.com/ZIMO-Elektronik/CMakeModules
  GIT_TAG v0.7.0
  SOURCE_DIR ${CMAKE_BINARY_DIR}/CMakeModules)
FetchContent_MakeAvailable(CMakeModules)

version_from_git()
project(
  DCC
  VERSION ${VERSION_FROM_GIT}
  LANGUAGES C CXX)

set(DCC_MAX_PACKET_SIZE
    18uz
    CACHE STRING "Maximum size of a packet in bytes")
set(DCC_RX_PREAMBLE_BITS
    10uz
    CACHE STRING "Number of preambel bits of decoder")
set(DCC_RX_DEQUE_SIZE
    32uz-1uz
    CACHE STRING "Size of the receiver deque of decoder")
set(DCC_RX_BIDI_DEQUE_SIZE
    8uz-1uz
    CACHE STRING "Size of the sender deque of decoder")
set(DCC_TX_MIN_PREAMBLE_BITS
    17uz
    CACHE STRING "Minimum number of preambel bits of command station")
set(DCC_TX_DEQUE_SIZE
    4uz-1uz
    CACHE STRING "Size of the transmitter deque of command station")

add_library(DCC INTERFACE)
add_library(DCC::DCC ALIAS DCC)

target_compile_features(DCC INTERFACE cxx_std_23)

target_compile_definitions(
  DCC
  INTERFACE DCC_MAX_PACKET_SIZE=${DCC_MAX_PACKET_SIZE}
            DCC_RX_PREAMBLE_BITS=${DCC_RX_PREAMBLE_BITS}
            DCC_RX_DEQUE_SIZE=${DCC_RX_DEQUE_SIZE}
            DCC_RX_BIDI_DEQUE_SIZE=${DCC_RX_BIDI_DEQUE_SIZE}
            DCC_TX_MIN_PREAMBLE_BITS=${DCC_TX_MIN_PREAMBLE_BITS}
            DCC_TX_DEQUE_SIZE=${DCC_TX_DEQUE_SIZE})

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  target_include_directories(DCC INTERFACE include)
  target_compile_options(DCC
                         INTERFACE -Werror;-Wno-error=deprecated-declarations)
else()
  target_include_directories(DCC SYSTEM INTERFACE include)
endif()

target_common_warnings(DCC INTERFACE)

if(NOT TARGET static_math)
  cpmaddpackage(
    NAME
    static_math
    GITHUB_REPOSITORY
    Morwenn/static_math
    GIT_TAG
    master
    SYSTEM
    YES
    OPTIONS
    "STATIC_MATH_BUILD_TESTS OFF")
endif()

if(NOT TARGET ZTL::ZTL)
  cpmaddpackage("gh:ZIMO-Elektronik/ZTL@0.19.0")
endif()

target_link_libraries(DCC INTERFACE static_math ZTL::ZTL)

if(PROJECT_IS_TOP_LEVEL)
  include(CTest)
  add_subdirectory(examples)
  file(
    DOWNLOAD
    https://raw.githubusercontent.com/ZIMO-Elektronik/.clang-format/master/.clang-format
    ${CMAKE_CURRENT_LIST_DIR}/.clang-format)
  file(GLOB_RECURSE SRC examples/*.[ch]pp include/*.[ch]pp src/*.[ch]pp
       tests/*.[ch]pp)
  add_clang_format_target(DCCFormat OPTIONS -i FILES ${SRC})
endif()

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
