# CMakeLists.txt --- CMake project settings
#    ex) cmake .
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 2.4)

# project name and language
project(TypicalX11App C CXX)

# set program information
set(PROGRAM_MAJOR 0)
set(PROGRAM_MINOR 1)
set(PROGRAM_BUILD 0)
set(PROGRAM_NAME "TypicalX11App")
set(PROGRAM_AUTHORS "Katayama Hirofumi MZ")
set(PROGRAM_YEARS "2015-2016")

# check build type
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# make uppercase string of build type
string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)

# check build type again
if (NOT uppercase_CMAKE_BUILD_TYPE MATCHES "^(DEBUG|RELEASE|RELWITHDEBINFO|MINSIZEREL)$")
    message(FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif()

# define _DEBUG on Debug version
if (uppercase_CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DEBUG")
endif()

# set output directory (build/)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# executable
add_executable(TypicalX11App TypicalX11App.cpp)

# link to X11
find_package(X11 REQUIRED)
target_link_libraries(TypicalX11App ${X11_LIBRARIES})

# link to Xpm
target_link_libraries(TypicalX11App Xpm)

# configuration files
configure_file(config.h.in config.h)

##############################################################################
