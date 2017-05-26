cmake_minimum_required(VERSION 3.7)
project(GenerativeArt)

set(CMAKE_CXX_STANDARD 14)

# production
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fpermissive -std=c++14 -O3 -Wall -Wextra -pedantic")
# debug
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fpermissive -std=c++14 -Wall -Wextra -pedantic")

# search for pkg-config
include (FindPkgConfig)
if (NOT PKG_CONFIG_FOUND)
    message (FATAL_ERROR "pkg-config not found")
endif ()

# check for libpng
pkg_check_modules (LIBPNG libpng12 REQUIRED)
if (NOT LIBPNG_FOUND)
    message(FATAL_ERROR "You don't seem to have libpng16 development libraries installed")
else ()
    include_directories (${LIBPNG_INCLUDE_DIRS})
    link_directories (${LIBPNG_LIBRARY_DIRS})
    link_libraries (${LIBPNG_LIBRARIES})
endif ()

include(FindOpenMP)
if(NOT OPENMP_FOUND)
    message("OpenMP not found. Trying '-fopenmp=libiomp5'")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp=libiomp5")
else()
    message("OpenMP found: ${OpenMP_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

include_directories(${PROJECT_SOURCE_DIR}/png++ ${PROJECT_SOURCE_DIR}/includes)

#set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")

add_executable(generate_samples sources/generate_samples.cpp ${LIBPNG_LINK_FLAGS})

add_executable(generate_high_res_image sources/generate_high_resolution_image.cpp ${LIBPNG_LINK_FLAGS})