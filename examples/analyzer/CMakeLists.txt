file(GLOB_RECURSE SRC *.cpp)
add_executable(DCCAnalyzer ${SRC})

target_common_warnings(DCCAnalyzer PRIVATE)
target_common_errors(DCCAnalyzer PRIVATE)

if(NOT TARGET imgui)
  cpmaddpackage(
    NAME
    imgui
    GITHUB_REPOSITORY
    "ocornut/imgui"
    VERSION
    1.92.4
    GIT_TAG
    v1.92.4-docking
    DOWNLOAD_ONLY
    TRUE)

  # imgui still doesn't come with a CMakeLists.txt *sigh*
  file(GLOB SRC ${imgui_SOURCE_DIR}/*.cpp)
  add_library(
    imgui STATIC ${SRC} ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
                 ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp)
  target_include_directories(
    imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends
                 ${imgui_SOURCE_DIR}/examples/libs)

  if(CMAKE_SYSTEM_NAME STREQUAL CMAKE_HOST_SYSTEM_NAME)
    # On host, build SDL2 and find OpenGL
    cpmaddpackage(
      NAME
      SDL
      VERSION
      2.32.6
      GIT_TAG
      release-2.32.6
      GITHUB_REPOSITORY
      "libsdl-org/SDL"
      SYSTEM
      YES)
    find_package(OpenGL REQUIRED)
  elseif(CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    # ... neither of those things are necessary for Emscripten
    set_target_properties(DCCAnalyzer PROPERTIES SUFFIX .html OUTPUT_NAME index)
    target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_ES2)
    target_compile_options(imgui PUBLIC "-sUSE_SDL=2")
    target_link_options(
      imgui
      PUBLIC
      "-sUSE_SDL=2"
      "-sFULL_ES3=1"
      "-sALLOW_MEMORY_GROWTH=1"
      "-sUSE_WEBGL2=1"
      "--shell-file=${CMAKE_CURRENT_LIST_DIR}/index.html")
  endif()

  target_link_libraries(
    imgui
    PUBLIC
      SDL2
      "$<$<STREQUAL:${CMAKE_SYSTEM_NAME},${CMAKE_HOST_SYSTEM_NAME}>:OpenGL::OpenGL>"
  )
endif()

target_link_libraries(DCCAnalyzer PRIVATE DCC::DCC imgui)
