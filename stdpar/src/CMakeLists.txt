cmake_minimum_required(VERSION 3.8)

project (LULESH)
find_package(TBB REQUIRED)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_COMPILER icpx)

if(NOT DEFINED ENABLE_STDPAR_REDIRECT)
    set(ENABLE_STDPAR_REDIRECT ON)
endif()

if(NOT DEFINED LULESH_USE_SYCL_USM)
    set(LULESH_USE_SYCL_USM OFF)
endif()

if(NOT DEFINED ENABLE_AOT)
    set(ENABLE_AOT OFF)
endif()

if(NOT DEFINED LULESH_PER_ALGO_TIME)
    set(LULESH_PER_ALGO_TIME OFF)
endif()

add_compile_options(-fsycl -O3 -w -DUSE_MPI=0)
add_link_options(-fsycl -w -ltbb)

if(ENABLE_AOT)
    add_compile_options(-fsycl-targets=spir64 -Xs "-device pvc")
endif()

if(ENABLE_STDPAR_REDIRECT)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../generated_headers)
    add_compile_options(-DSTDPAR -fsycl-unnamed-lambda -cl-mad-enable -ffp-contract=fast)
    if(NOT LULESH_USE_SYCL_USM)
        add_link_options(-loverusm)
    endif()
endif()

if(LULESH_USE_SYCL_USM)
    add_compile_options(-DLULESH_USE_SYCL_USM)
endif()

if(LULESH_PER_ALGO_TIME)
    add_compile_options(-DLULESH_PER_ALGO_TIME)
endif()

add_executable(lulesh lulesh.cc lulesh-comm.cc lulesh-viz.cc lulesh-util.cc lulesh-init.cc)
