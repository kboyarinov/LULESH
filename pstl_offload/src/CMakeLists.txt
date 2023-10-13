cmake_minimum_required(VERSION 3.8)

project (LULESH)
find_package(TBB REQUIRED)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_COMPILER icpx)

if(NOT DEFINED ENABLE_PSTL_OFFLOAD)
    set(ENABLE_PSTL_OFFLOAD OFF)
endif()

if(NOT DEFINED LULESH_USE_SYCL_USM)
    set(LULESH_USE_SYCL_USM OFF)
endif()

if(NOT DEFINED ENABLE_AOT)
    set(ENABLE_AOT OFF)
endif()

if(NOT DEFINED DPL_PATH)
    message(FATAL_ERROR "DPL_PATH should be passed to CMake")
endif()

if(NOT DEFINED LULESH_ALLOC_CHANGE)
    set(LULESH_ALLOC_CHANGE OFF)
endif()

add_compile_options(-fsycl -O3 -w -DUSE_MPI=0)
add_link_options(-fsycl -w)

if(ENABLE_AOT)
    add_compile_options(-fsycl-targets=spir64 -Xs "-device pvc")
endif()

if(ENABLE_PSTL_OFFLOAD)
    # include_directories(${DPL_PATH}/include/pstl_offload)
    # add_compile_options(-D__SYCL_PSTL_OFFLOAD__=1 -fsycl-unnamed-lambda -cl-mad-enable -ffp-contract=fast)
    # if(NOT LULESH_USE_SYCL_USM)
    #     add_link_options(-lpstloffload)
    # endif()
    add_compile_options(-fsycl-pstl-offload=gpu -cl-mad-enable -ffp-contract=fast)
    add_link_options(-fsycl-pstl-offload=gpu)
else()
    add_link_options(-ltbb)
endif()

if(LULESH_USE_SYCL_USM)
    add_compile_options(-DLULESH_USE_SYCL_USM)
endif()

if(LULESH_ALLOC_CHANGE)
    add_compile_options(-DLULESH_ALLOCATE_EXP)
endif()

include_directories(${DPL_PATH}/include)

add_executable(lulesh lulesh.cc lulesh-comm.cc lulesh-viz.cc lulesh-util.cc lulesh-init.cc)
