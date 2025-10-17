# README:
# This is the CMake configuration file for this game project.
# See the file build.bat or build.sh for info on how to build with CMake.

# Project Settings
# --------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+

# Name of the project, using C
project("SlapMaster" C)

# Executable name
set(OUTPUT_NAME slapmaster)

set(LIBRARIES raylib)
if(NOT MSVC) # math library for Unix
  list(APPEND LIBRARIES m)
endif()

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
# --------------------------------------------------------------------------------

# Download and build raylib if needed
# set(raylib_VERBOSE 1) # Uncomment this for more detailed raylib build output
set(RAYLIB_VERSION 5.5)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
  include(FetchContent)
  FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_MakeAvailable(raylib)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
  endif()
endif()

# Setup Project
# --------------------------------------------------------------------------------

file(GLOB SRC_FILES src/*.c src/entity/*.c)
add_executable(${OUTPUT_NAME} ${SRC_FILES})
target_include_directories(${OUTPUT_NAME} PRIVATE src/include)
target_link_libraries(${OUTPUT_NAME} ${LIBRARIES})

# Cross-platform Configurations
# --------------------------------------------------------------------------------

# Windows / Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} # startup project set to game
  PROPERTY VS_STARTUP_PROJECT ${OUTPUT_NAME})
set_target_properties(${OUTPUT_NAME} # working directory set to repo directory
  PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ./)

# Web
if (${PLATFORM} STREQUAL "Web")
  set_target_properties(${OUTPUT_NAME} PROPERTIES SUFFIX ".html")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wno-missing-braces -Wunused-result -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wfloat-conversion")
  set(CMAKE_C_FLAGS_RELEASE "-Os" CACHE STRING "" FORCE)
  set(CMAKE_EXE_LINKER_FLAGS "--shell-file ${CMAKE_SOURCE_DIR}/shell.html -sUSE_GLFW=3 -sFORCE_FILESYSTEM=1 -sASYNCIFY -sTOTAL_MEMORY=67108864 -sEXPORTED_FUNCTIONS=_main,requestFullscreen -sEXPORTED_RUNTIME_METHODS=HEAPF32 --preload-file ${CMAKE_SOURCE_DIR}/assets@assets")
endif()

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if (APPLE)
  target_link_libraries(${OUTPUT_NAME} "-framework IOKit")
  target_link_libraries(${OUTPUT_NAME} "-framework Cocoa")
  target_link_libraries(${OUTPUT_NAME} "-framework OpenGL")
endif()
