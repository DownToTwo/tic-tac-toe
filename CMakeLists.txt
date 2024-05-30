cmake_minimum_required(VERSION 3.11)

# Set the default build type to Release if not specified
if (NOT EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
  if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
  endif()
endif()

# Set compiler flags
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O2 -s -flto=auto")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-O -Wformat -Wformat-signedness -Wall -Wextra -g3 -Wpedantic -fsanitize=address,leak,undefined -Wcast-qual -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code -Wwrite-strings -Wshadow -Wpointer-arith -Wcast-align -Wfloat-equal -Wundef -fverbose-asm -Weffc++ -Wpadded")

if (EMSCRIPTEN)
  string(APPEND CMAKE_CXX_FLAGS " -s USE_PTHREADS")
  string(APPEND CMAKE_EXE_LINKER_FLAGS " -s ASYNCIFY -s PROXY_TO_PTHREAD")
endif()

# Fetch FTXUI and ftxui-grid-container
include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)

FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v5.0.0
)

FetchContent_Declare(
  ftxui-grid-container
  GIT_REPOSITORY https://github.com/mingsheng13/grid-container-ftxui
  GIT_TAG master
)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(ftxui ftxui-grid-container nlohmann_json)

# Project configuration
project(tic-tac-toe LANGUAGES CXX VERSION 1.0.0)

# Add executable
add_executable(tic-tac-toe src/main.cpp)
target_include_directories(tic-tac-toe PRIVATE src)
set_target_properties(tic-tac-toe PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)

# Link libraries
target_link_libraries(tic-tac-toe
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui-grid-container
  PRIVATE nlohmann_json::nlohmann_json
  )

# Additional configuration for Emscripten
if (EMSCRIPTEN)
  foreach(file "index.html" "run_webassembly.py")
    configure_file("src/${file}" ${file})
  endforeach()
endif()
