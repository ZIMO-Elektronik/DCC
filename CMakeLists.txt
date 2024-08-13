cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
include(FetchContent)

if(ESP_PLATFORM)
  file(GLOB_RECURSE SRC src/*.c)
  idf_component_register(SRCS ${SRC} INCLUDE_DIRS include REQUIRES driver)
  # Change OUTPUT_NAME from DCC to lib__idf_DCC to avoid conflicts
  set_target_properties(${COMPONENT_LIB} PROPERTIES PREFIX "")
  set_target_properties(
    ${COMPONENT_LIB} PROPERTIES OUTPUT_NAME
                                ${CMAKE_STATIC_LIBRARY_PREFIX}${COMPONENT_LIB})
  target_link_libraries(${COMPONENT_LIB} PUBLIC DCC)
endif()

FetchContent_Declare(
  CMakeModules
  GIT_REPOSITORY "https://github.com/ZIMO-Elektronik/CMakeModules"
  GIT_TAG v0.9.2
  SOURCE_DIR ${CMAKE_BINARY_DIR}/CMakeModules)
FetchContent_MakeAvailable(CMakeModules)

version_from_git()
project(
  DCC
  VERSION ${VERSION_FROM_GIT}
  LANGUAGES ASM C CXX)

set(DCC_MAX_PACKET_SIZE
    18u
    CACHE STRING "Maximum size of a packet in bytes")
set(DCC_RX_MIN_BIT_1_TIMING
    52u
    CACHE STRING "Minimum duration of a 1 bit of decoder")
set(DCC_RX_MAX_BIT_1_TIMING
    64u
    CACHE STRING "Maximum duration of a 1 bit of decoder")
set(DCC_RX_MIN_BIT_0_TIMING
    90u
    CACHE STRING "Minimum duration of a 0 bit of decoder")
set(DCC_RX_MAX_BIT_0_TIMING
    119u
    CACHE STRING "Maximum duration of a 0 bit of decoder")
set(DCC_RX_MIN_PREAMBLE_BITS
    10u
    CACHE STRING "Minimum number of preambel bits of decoder")
set(DCC_RX_DEQUE_SIZE
    31u
    CACHE STRING "Size of the receiver deque of decoder")
set(DCC_RX_BIDI_DEQUE_SIZE
    7u
    CACHE STRING "Size of the sender deque of decoder")
set(DCC_TX_MIN_BIT_1_TIMING
    56u
    CACHE STRING "Minimum duration of a 1 bit of command station")
set(DCC_TX_MAX_BIT_1_TIMING
    60u
    CACHE STRING "Maximum duration of a 1 bit of command station")
set(DCC_TX_MIN_BIT_0_TIMING
    97u
    CACHE STRING "Minimum duration of a 0 bit of command station")
set(DCC_TX_MAX_BIT_0_TIMING
    114u
    CACHE STRING "Maximum duration of a 0 bit of command station")
set(DCC_TX_MIN_PREAMBLE_BITS
    17u
    CACHE STRING "Minimum number of preambel bits of command station")
set(DCC_TX_MAX_PREAMBLE_BITS
    30u
    CACHE STRING "Maximum number of preambel bits of command station")
set(DCC_TX_DEQUE_SIZE
    3u
    CACHE STRING "Size of the transmitter deque of command station")

add_library(DCC INTERFACE)
add_library(DCC::DCC ALIAS DCC)

target_compile_features(DCC INTERFACE cxx_std_23)

target_compile_definitions(
  DCC
  INTERFACE DCC_MAX_PACKET_SIZE=${DCC_MAX_PACKET_SIZE}
            DCC_RX_MIN_BIT_1_TIMING=${DCC_RX_MIN_BIT_1_TIMING}
            DCC_RX_MAX_BIT_1_TIMING=${DCC_RX_MAX_BIT_1_TIMING}
            DCC_RX_MIN_BIT_0_TIMING=${DCC_RX_MIN_BIT_0_TIMING}
            DCC_RX_MAX_BIT_0_TIMING=${DCC_RX_MAX_BIT_0_TIMING}
            DCC_RX_MIN_PREAMBLE_BITS=${DCC_RX_MIN_PREAMBLE_BITS}
            DCC_RX_DEQUE_SIZE=${DCC_RX_DEQUE_SIZE}
            DCC_RX_BIDI_DEQUE_SIZE=${DCC_RX_BIDI_DEQUE_SIZE}
            DCC_TX_MIN_BIT_1_TIMING=${DCC_TX_MIN_BIT_1_TIMING}
            DCC_TX_MAX_BIT_1_TIMING=${DCC_TX_MAX_BIT_1_TIMING}
            DCC_TX_MIN_BIT_0_TIMING=${DCC_TX_MIN_BIT_0_TIMING}
            DCC_TX_MAX_BIT_0_TIMING=${DCC_TX_MAX_BIT_0_TIMING}
            DCC_TX_MIN_PREAMBLE_BITS=${DCC_TX_MIN_PREAMBLE_BITS}
            DCC_TX_MAX_PREAMBLE_BITS=${DCC_TX_MAX_PREAMBLE_BITS}
            DCC_TX_DEQUE_SIZE=${DCC_TX_DEQUE_SIZE})

if(PROJECT_IS_TOP_LEVEL)
  target_include_directories(DCC INTERFACE include)
else()
  target_include_directories(DCC SYSTEM INTERFACE include)
endif()

target_common_warnings(DCC INTERFACE)

if(NOT TARGET static_math)
  cpmaddpackage(
    NAME
    static_math
    GITHUB_REPOSITORY
    "Morwenn/static_math"
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
    "https://github.com/ZIMO-Elektronik/.github/raw/master/data/.clang-format"
    ${CMAKE_CURRENT_LIST_DIR}/.clang-format)
  file(GLOB_RECURSE SRC examples/*.[ch]pp include/*.[ch]pp src/*.[ch]pp
       tests/*.[ch]pp)
  add_clang_format_target(DCCFormat OPTIONS -i FILES ${SRC})
endif()

if(BUILD_TESTING AND CMAKE_SYSTEM_NAME STREQUAL CMAKE_HOST_SYSTEM_NAME)
  add_subdirectory(tests)
endif()
