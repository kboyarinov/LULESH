cmake_minimum_required(VERSION 3.8)

project (LULESH)
find_package(TBB REQUIRED)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_COMPILER icpx)

if(NOT DEFINED ENABLE_STDPAR_REDIRECT)
    set(ENABLE_STDPAR_REDIRECT ON)
endif()

if(NOT DEFINED LULESH_STDPAR_POLICY)
    set(LULESH_STDPAR_POLICY par_unseq)
endif()

if(NOT DEFINED LULESH_DEBUG_PRINTS)
    set(LULESH_DEBUG_PRINTS OFF)
endif()

if(NOT DEFINED LULESH_MEASURE_EACH)
    set(LULESH_MEASURE_EACH OFF)
endif()

if(NOT DEFINED LULESH_USE_ONEDPL)
    set(LULESH_USE_ONEDPL OFF)
endif()

if(NOT DEFINED LULESH_USE_SYCL_USM)
    set(LULESH_USE_SYCL_USM OFF)
endif()

if(NOT DEFINED LULESH_USE_ITERATORS)
    set(LULESH_USE_ITERATORS OFF)
endif()

add_compile_options(-fsycl -O3 -w -DUSE_MPI=0 -DLULESH_STDPAR_POLICY=${LULESH_STDPAR_POLICY})
add_link_options(-fsycl -w -ltbb)

if(ENABLE_STDPAR_REDIRECT)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../generated_headers)
    add_compile_options(-DSTDPAR -fsycl-unnamed-lambda
-cl-mad-enable -ffp-contract=fast)
    if(!LULESH_USE_SYCL_USM)
        target_link_libraries(overusm)
    endif()
endif()

if(LULESH_DEBUG_PRINTS)
    add_compile_options(-DSTDPAR_DEBUG)
endif()

if(LULESH_MEASURE_EACH)
    add_compile_options(-DMEASURE_EACH_ALGORITHM)
endif()

if(LULESH_USE_ONEDPL)
    add_compile_options(-DUSE_ONEDPL)
endif()

if(LULESH_USE_SYCL_USM)
    add_compile_options(-DUSE_USM_VECTOR)
endif()

if(LULESH_USE_ITERATORS)
    add_compile_options(-DUSE_ITERATORS)
endif()

add_executable(lulesh lulesh.cc lulesh-comm.cc lulesh-viz.cc lulesh-util.cc lulesh-init.cc)
