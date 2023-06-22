cmake_minimum_required(VERSION 3.14)

# set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsycl")

project (over LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 17)

find_package(TBB REQUIRED)
get_target_property(tbb_header_path TBB::tbbmalloc INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "Using TBB header path ${tbb_header_path}")
unset(tbb_header_path)

set(DPL_PATH $ENV{DPL_ROOT} CACHE PATH "DPL path")
message(STATUS "Using DPL from ${DPL_PATH}/include")
include_directories(${DPL_PATH}/include)

# find_package(IntelSYCL REQUIRED)

add_compile_options(-fsycl -Wall)

add_library(overusm SHARED src/proxy.cpp src/safer.cpp
	src/function_replacement.cpp)

target_link_libraries(overusm TBB::tbbmalloc)
