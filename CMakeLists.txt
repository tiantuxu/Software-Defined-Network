#cmake_minimum_required(VERSION 3.9)
project(ece50863_project1)

set(CMAKE_CXX_STANDARD 11)

set(deploy_include_path "/shared/include")

################################
#  toolchain, dependency, etc.
################################

SET( CMAKE_CXX_COMPILER "/usr/bin/g++-5"  )
SET( CMAKE_C_COMPILER "gcc"  )
#SET( CMAKE_CXX_COMPILER "icc" )

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=auto")

SET(CMAKE_CXX_FLAGS_DEBUG
        "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11 -g -O0 -Wall -Wfatal-errors")
SET(CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -g -Wall -O3 -DHAVE_NUMA")

SET(CMAKE_C_FLAGS_DEBUG
        "${CMAKE_C_FLAGS_DEBUG} -std=gnu11 -g -O0 -Wall -Wfatal-errors")

set(deploy_target_path "localhost:/tmp/$ENV{USER}")

file (GLOB LIB_SOURCES
        ${HEADERS}
        controller.cpp
        switch.cpp
        )

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    #add_definitions(-D_GLIBCXX_DEBUG) # if the compliation unit use boost::, it's gonna break.
    add_definitions(-DCONFIG_KAGE_GLOBAL_DEBUG_LEVEL=30)
    add_definitions(-DDEBUG)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DCONFIG_KAGE_GLOBAL_DEBUG_LEVEL=50)
    add_definitions(-DNDEBUG)
endif ()

add_executable(controller.bin controller.cpp message.h graph.h dijkstra.cpp)
add_executable(switch.bin switch.cpp message.h)