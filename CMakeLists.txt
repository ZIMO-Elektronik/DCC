cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
include(FetchContent)

if(ESP_PLATFORM)
  # >5.3.0 requires esp_driver_rmt
  if(IDF_VERSION_MAJOR GREATER 5 OR (IDF_VERSION_MAJOR EQUAL 5
                                     AND IDF_VERSION_MINOR GREATER_EQUAL 3))
    set(ESP_DRIVER_RMT_COMPONENT esp_driver_rmt)
  endif()
  idf_component_register(
    SRCS
    src/rmt_dcc_encoder.c
    INCLUDE_DIRS
    include
    REQUIRES
    driver
    ${ESP_DRIVER_RMT_COMPONENT})

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
  GIT_TAG v0.12.2
  SOURCE_DIR ${CMAKE_BINARY_DIR}/CMakeModules)
FetchContent_MakeAvailable(CMakeModules)

version_from_git()
project(
  DCC
  VERSION ${VERSION_FROM_GIT}
  LANGUAGES ASM C CXX)

option(DCC_STANDARD_COMPLIANCE "Standard compliance" OFF)
set(DCC_MANUFACTURER_ID
    145u
    CACHE STRING "Manufacturer ID")
set(DCC_MAX_PACKET_SIZE
    18u
    CACHE STRING "Maximum size of a packet in bytes")
set(DCC_RX_LOGON_DID_CV_ADDRESS
    249u
    CACHE STRING "First CV address of decoder ID")
set(DCC_RX_LOGON_CID_CV_ADDRESS
    65296u
    CACHE STRING "First CV address of last CID")
set(DCC_RX_LOGON_SID_CV_ADDRESS
    65298u
    CACHE STRING "First CV address of last SID")
set(DCC_RX_LOGON_ADDRESS_CV_ADDRESS
    65299u
    CACHE STRING "First CV address of last logon address")
set(DCC_RX_MIN_PREAMBLE_BITS
    10u
    CACHE STRING "Minimum number of preamble bits of decoder")
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
set(DCC_RX_DEQUE_SIZE
    31u
    CACHE STRING "Size of the receiver deque of decoder")
set(DCC_RX_BIDI_DEQUE_SIZE
    7u
    CACHE STRING "Size of the sender deque of decoder")
set(DCC_TX_MIN_PREAMBLE_BITS
    17u
    CACHE STRING "Minimum number of preamble bits of command station")
set(DCC_TX_MAX_PREAMBLE_BITS
    30u
    CACHE STRING "Maximum number of preamble bits of command station")
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
set(DCC_TX_MIN_BIDI_BIT_TIMING
    57u
    CACHE STRING "Minimum duration of a BiDi bit of command station")
set(DCC_TX_MAX_BIDI_BIT_TIMING
    61u
    CACHE STRING "Maximum duration of a BiDi bit of command station")
set(DCC_TX_DEQUE_SIZE
    3u
    CACHE STRING "Size of the transmitter deque of command station")

add_library(DCC INTERFACE)
add_library(DCC::DCC ALIAS DCC)

target_compile_features(DCC INTERFACE cxx_std_23)

target_compile_definitions(
  DCC
  INTERFACE DCC_STANDARD_COMPLIANCE=$<BOOL:${DCC_STANDARD_COMPLIANCE}>
            DCC_MANUFACTURER_ID=${DCC_MANUFACTURER_ID}
            DCC_MAX_PACKET_SIZE=${DCC_MAX_PACKET_SIZE}
            DCC_RX_LOGON_DID_CV_ADDRESS=${DCC_RX_LOGON_DID_CV_ADDRESS}
            DCC_RX_LOGON_CID_CV_ADDRESS=${DCC_RX_LOGON_CID_CV_ADDRESS}
            DCC_RX_LOGON_SID_CV_ADDRESS=${DCC_RX_LOGON_SID_CV_ADDRESS}
            DCC_RX_LOGON_ADDRESS_CV_ADDRESS=${DCC_RX_LOGON_ADDRESS_CV_ADDRESS}
            DCC_RX_MIN_PREAMBLE_BITS=${DCC_RX_MIN_PREAMBLE_BITS}
            DCC_RX_MIN_BIT_1_TIMING=${DCC_RX_MIN_BIT_1_TIMING}
            DCC_RX_MAX_BIT_1_TIMING=${DCC_RX_MAX_BIT_1_TIMING}
            DCC_RX_MIN_BIT_0_TIMING=${DCC_RX_MIN_BIT_0_TIMING}
            DCC_RX_MAX_BIT_0_TIMING=${DCC_RX_MAX_BIT_0_TIMING}
            DCC_RX_DEQUE_SIZE=${DCC_RX_DEQUE_SIZE}
            DCC_RX_BIDI_DEQUE_SIZE=${DCC_RX_BIDI_DEQUE_SIZE}
            DCC_TX_MIN_PREAMBLE_BITS=${DCC_TX_MIN_PREAMBLE_BITS}
            DCC_TX_MAX_PREAMBLE_BITS=${DCC_TX_MAX_PREAMBLE_BITS}
            DCC_TX_MIN_BIT_1_TIMING=${DCC_TX_MIN_BIT_1_TIMING}
            DCC_TX_MAX_BIT_1_TIMING=${DCC_TX_MAX_BIT_1_TIMING}
            DCC_TX_MIN_BIT_0_TIMING=${DCC_TX_MIN_BIT_0_TIMING}
            DCC_TX_MAX_BIT_0_TIMING=${DCC_TX_MAX_BIT_0_TIMING}
            DCC_TX_MIN_BIDI_BIT_TIMING=${DCC_TX_MIN_BIDI_BIT_TIMING}
            DCC_TX_MAX_BIDI_BIT_TIMING=${DCC_TX_MAX_BIDI_BIT_TIMING}
            DCC_TX_DEQUE_SIZE=${DCC_TX_DEQUE_SIZE})

# https://github.com/espressif/esp-idf/issues/17773
if(PROJECT_IS_TOP_LEVEL AND NOT ESP_PLATFORM)
  target_include_directories(DCC INTERFACE include)
  target_common_warnings(DCC INTERFACE)
  target_common_errors(DCC INTERFACE -Wfatal-errors)
else()
  target_include_directories(DCC SYSTEM INTERFACE include)
endif()

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
    "STATIC_MATH_BUILD_TESTS OFF"
    "CMAKE_POLICY_VERSION_MINIMUM 3.5")
endif()

if(NOT TARGET ZTL::ZTL)
  cpmaddpackage("gh:ZIMO-Elektronik/ZTL@0.24.0")
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
  add_include_what_you_must_target(DCCIncludeWhatYouMust TARGET DCC
                                   EXCLUDE_HEADERS "rmt_dcc_encoder.h")
endif()

if(BUILD_TESTING
   AND PROJECT_IS_TOP_LEVEL
   AND CMAKE_SYSTEM_NAME STREQUAL CMAKE_HOST_SYSTEM_NAME)
  add_subdirectory(tests)
endif()
