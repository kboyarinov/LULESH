cmake_minimum_required(VERSION 3.8)

project (LULESH)
find_package(TBB REQUIRED)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_COMPILER icpx)

if(NOT DEFINED ENABLE_STDPAR_REDIRECT)
    set(ENABLE_STDPAR_REDIRECT ON)
endif()

add_compile_options(-fsycl -O3 -w -DUSE_MPI=0)
add_link_options(-fsycl -w -ltbb)

if(ENABLE_STDPAR_REDIRECT)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../generated_headers)
    add_compile_options(-DSTDPAR -fsycl-unnamed-lambda -cl-mad-enable -ffp-contract=fast)
    # add_link_options(-loverusm)
endif()

add_executable(lulesh lulesh.cc lulesh-comm.cc lulesh-viz.cc lulesh-util.cc lulesh-init.cc)
