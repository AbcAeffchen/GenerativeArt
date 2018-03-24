cmake_minimum_required(VERSION 3.7)
project(GenerativeArt)

set(CMAKE_CXX_STANDARD 14)

# production
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fpermissive -std=c++14 -O3 -Wall -Wextra -pedantic -Wno-unused-parameter")

# search for pkg-config
include (FindPkgConfig)
if(NOT PKG_CONFIG_FOUND)
    message (FATAL_ERROR "pkg-config not found")
endif()

# check for libpng
pkg_check_modules (LIBPNG libpng12 REQUIRED)
if (NOT LIBPNG_FOUND)
    message(FATAL_ERROR "You don't seem to have libpng16 development libraries installed")
else()
    include_directories (${LIBPNG_INCLUDE_DIRS})
    link_directories (${LIBPNG_LIBRARY_DIRS})
    link_libraries (${LIBPNG_LIBRARIES})
endif()

include(FindOpenMP)
if(NOT OPENMP_FOUND)
    message("OpenMP not found. Trying '-fopenmp=libiomp5'")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp=libiomp5")
else()
    message("OpenMP found: ${OpenMP_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

include_directories(${PROJECT_SOURCE_DIR}/includes
        ${PROJECT_SOURCE_DIR}/CLI11
        ${PROJECT_SOURCE_DIR}/png++
        /opt/Vc/include/)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
endif()

set(SOURCES sources/main.cpp sources/FunctionPool.cpp)
add_executable(GenerativeArt ${SOURCES} ${LIBPNG_LINK_FLAGS})
