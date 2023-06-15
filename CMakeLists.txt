cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
include(FetchContent)

if(NOT CMAKE_BUILD_EARLY_EXPANSION)
  set(FETCHCONTENT_QUIET FALSE)
  FetchContent_Declare(
    CMakeModules
    GIT_REPOSITORY https://github.com/ZIMO-Elektronik/CMakeModules
    GIT_TAG v0.0.5
    SOURCE_DIR ${CMAKE_BINARY_DIR}/CMakeModules)
  FetchContent_MakeAvailable(CMakeModules)

  project(
    DCC
    VERSION 0.27.0
    LANGUAGES C CXX)
endif()

set(DCC_MAX_PACKET_SIZE
    18uz
    CACHE STRING "Maximum size of a packet in bytes")
set(DCC_RX_PREAMBLE_BITS
    10uz
    CACHE STRING "Number of preambel bits of decoder")
set(DCC_RX_QUEUE_SIZE
    32uz-1uz
    CACHE STRING "Size of the receiver queue of decoder")
set(DCC_RX_BIDI_QUEUE_SIZE
    8uz-1uz
    CACHE STRING "Size of the sender queue of decoder")
set(DCC_TX_MIN_PREAMBLE_BITS
    17uz
    CACHE STRING "Minimum number of preambel bits of command station")
set(DCC_TX_QUEUE_SIZE
    4uz-1uz
    CACHE STRING "Size of the transmitter queue of command station")

if(ESP_PLATFORM)
  file(GLOB_RECURSE SRC src/*.c)
  idf_component_register(SRCS ${SRC} INCLUDE_DIRS include REQUIRES driver)
  idf_build_get_property(TGT IDF_TARGET)
  string(TOUPPER ${TGT} TGT)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE CONFIG_IDF_TARGET_${TGT})
  target_link_libraries(${COMPONENT_LIB} PUBLIC DCC)
endif()

add_library(DCC INTERFACE)
add_library(DCC::DCC ALIAS DCC)

target_compile_features(DCC INTERFACE cxx_std_23)

target_compile_definitions(
  DCC
  INTERFACE DCC_MAX_PACKET_SIZE=${DCC_MAX_PACKET_SIZE}
            DCC_RX_PREAMBLE_BITS=${DCC_RX_PREAMBLE_BITS}
            DCC_RX_QUEUE_SIZE=${DCC_RX_QUEUE_SIZE}
            DCC_RX_BIDI_QUEUE_SIZE=${DCC_RX_BIDI_QUEUE_SIZE}
            DCC_TX_MIN_PREAMBLE_BITS=${DCC_TX_MIN_PREAMBLE_BITS}
            DCC_TX_QUEUE_SIZE=${DCC_TX_QUEUE_SIZE})

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  target_include_directories(DCC INTERFACE include)
else()
  target_include_directories(DCC SYSTEM INTERFACE include)
endif()

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
cpmaddpackage("gh:ZIMO-Elektronik/ZTL@0.16.2")

target_link_libraries(DCC INTERFACE static_math ZTL::ZTL)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
endif()

if(BUILD_TESTING
   AND CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME
   AND CMAKE_SYSTEM_NAME STREQUAL CMAKE_HOST_SYSTEM_NAME)
  add_subdirectory(examples)
  add_subdirectory(tests)
endif()