cmake_minimum_required(VERSION 3.8)

project (over)

set (CMAKE_CXX_STANDARD 17)

add_compile_options(-fsycl -Wall)

# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(overusm SHARED src/proxy.cpp src/safer.cpp)

target_link_libraries(overusm tbbmalloc_debug)
