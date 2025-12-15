file(GLOB_RECURSE SRC src/*.cpp)
add_executable(DCCypher ${SRC})

target_compile_definitions(
  DCCypher PRIVATE DCC_CYPHER_VERSION="${PROJECT_VERSION}"
                   IMGUI_DEFINE_MATH_OPERATORS)

target_common_warnings(DCCypher PRIVATE)
target_common_errors(DCCypher PRIVATE)

# imgui still doesn't come with a CMakeLists.txt *sigh*
cpmaddpackage(
  NAME
  imgui
  GITHUB_REPOSITORY
  "ocornut/imgui"
  VERSION
  1.91.9b
  GIT_TAG
  v1.91.9b-docking
  DOWNLOAD_ONLY
  TRUE)

file(GLOB SRC ${imgui_SOURCE_DIR}/*.cpp)
add_library(
  imgui STATIC ${SRC} ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
               ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp)
target_include_directories(
  imgui SYSTEM PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends
                      ${imgui_SOURCE_DIR}/examples/libs)

if(CMAKE_SYSTEM_NAME STREQUAL CMAKE_HOST_SYSTEM_NAME)
  # On host, build SDL2 and find OpenGL
  cpmaddpackage(
    NAME
    SDL
    VERSION
    2.32.8
    GIT_TAG
    release-2.32.8
    GITHUB_REPOSITORY
    "libsdl-org/SDL"
    SYSTEM
    YES)
  find_package(OpenGL REQUIRED)
elseif(CMAKE_SYSTEM_NAME STREQUAL Emscripten)
  # ... neither of those things are necessary for Emscripten
  set_target_properties(DCCypher PROPERTIES SUFFIX .html OUTPUT_NAME index)
  target_compile_definitions(
    imgui
    PUBLIC IMGUI_IMPL_OPENGL_ES2
           # Default definition ain't compatible with Emscripten
           ImTextureID=unsigned\ int)
  target_compile_options(imgui PUBLIC "-sUSE_SDL=2")
  target_link_options(
    imgui
    PUBLIC
    "--shell-file=${CMAKE_CURRENT_LIST_DIR}/index.html"
    "-sALLOW_MEMORY_GROWTH=1"
    "-sFULL_ES3=1"
    "-sUSE_SDL=2"
    "-sUSE_WEBGL2=1")
endif()

target_link_libraries(
  imgui
  PUBLIC
    SDL2
    "$<$<STREQUAL:${CMAKE_SYSTEM_NAME},${CMAKE_HOST_SYSTEM_NAME}>:OpenGL::OpenGL>"
)

# Only supports imgui < 1.92.0
cpmaddpackage(
  NAME
  implot
  GITHUB_REPOSITORY
  "epezent/implot"
  VERSION
  0.16
  DOWNLOAD_ONLY
  TRUE)
file(GLOB SRC ${implot_SOURCE_DIR}/*.cpp)
add_library(implot STATIC ${SRC})
target_include_directories(implot SYSTEM PUBLIC ${implot_SOURCE_DIR})
target_link_libraries(implot PUBLIC imgui)

target_link_libraries(DCCypher PRIVATE DCC::DCC imgui implot)
