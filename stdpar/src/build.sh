#!/bin/bash

if [ "$1" == "-hg" ]
then
    echo "Usage"
    echo "./build.sh <USE_STDPAR> <STDPAR_POLICY> <DEBUG> <MEASURE_EACH_ALGORITHM> <USE_ONEDPL> <USE_SYCL_USM>"
    echo "    <USE_STDPAR> - ON if usage of STDPAR prototype is intended, OFF otherwise, ON by default"
    echo "    <STDPAR_POLICY> - policy for standard parallelism, par_unseq by default"
    echo "    <DEBUG> - ON to print debug messages, OFF otherwise, OFF by default"
    echo "    <MEASURE_EACH_ALGORITHM> - ON to print execution time of each algorithm, OFF otherwise, OFF by default"
    echo "    <USE_ONEDPL> - ON to use oneDPL algorithms with GPU policy, OFF otherwise, OFF by default"
    echo "    <USE_SYCL_USM> - ON to use manual shared memory allocation, OFF otherwise, OFF by default"
    exit 0
fi

STDPAR="$1"

if [ -z "$1" ]
then
    echo "GPU STDPAR not specified - ON by default"
    STDPAR="ON"
fi

LULESH_STDPAR_POLICY="$2"

if [ -z "$LULESH_STDPAR_POLICY" ]
then
    echo "Policy not specified - par_unseq by default"
    LULESH_STDPAR_POLICY="par_unseq"
fi

DEBUG="$3"

if [ -z "$DEBUG" ]
then
    echo "Debug not specified - OFF by default"
    DEBUG="OFF"
fi

MEASURE_EACH="$4"

if [ -z "$MEASURE_EACH" ]
then
    echo "Measure each is not specified - OFF by default"
    MEASURE_EACH="OFF"
fi

USE_ONEDPL="$5"

if [ -z "$USE_ONEDPL" ]
then
    echo "USE_ONEDPL is OFF by default"
    USE_ONEDPL="OFF"
fi

USE_USM_VECTOR="$6"

if [ -z "USE_USM_VECTOR" ]
then
    echo "USE_USM_VECTOR is OFF by default"
    USE_USM_VECTOR="OFF"
fi

GENERATED_HEADERS="/tmp/kboyarin/LULESH/generated_headers"
DPL_INCLUDE="/tmp/kboyarin/oneDPL/include"

CXX_FLAGS="-DLULESH_STDPAR_POLICY=$LULESH_STDPAR_POLICY"
LINK_FLAGS=""
OPT_LEVEL="3"

if [ "$STDPAR" == "ON" ]
then
    CXX_FLAGS+=" -DSTDPAR -I $GENERATED_HEADERS"
    LINK_FLAGS+=" -loverusm"
fi

if [ "$DEBUG" == "ON" ]
then
    CXX_FLAGS+=" -g -DSTDPAR_DEBUG"
    OPT_LEVEL="0"
fi

if [ "$MEASURE_EACH" == "ON" ]
then
    CXX_FLAGS+=" -DMEASURE_EACH_ALGORITHM"
fi

if [ "$USE_ONEDPL" == "ON" ]
then
    CXX_FLAGS+=" -DUSE_ONEDPL"
fi

if [ "$USE_USM_VECTOR" == "ON" ]
then
    CXX_FLAGS+=" -DUSE_USM_VECTOR"
fi

CXX_FLAGS+=" -O$OPT_LEVEL"
LINK_FLAGS+=" -O$OPT_LEVEL"

echo "building lulesh: GPU STDPAR = $STDPAR LULESH_STDPAR_POLICY = $LULESH_STDPAR_POLICY DEBUG = $DEBUG MEASURE EACH = $MEASURE_EACH"

BUILD_LULESH_CC_COMMPAND="icpx -fsycl -c -w -std=c++17 -DUSE_MPI=0 $CXX_FLAGS -I $DPL_INCLUDE -o lulesh.o lulesh.cc"
BUILD_LULESH_COMM_CC_COMMAND="icpx -fsycl -c -w -std=c++17 -DUSE_MPI=0 $CXX_FLAGS -I $DPL_INCLUDE -o lulesh-comm.o lulesh-comm.cc"
BUILD_LULESH_VIZ_CC_COMMAND="icpx -fsycl -c -w -std=c++17 -DUSE_MPI=0 $CXX_FLAGS -I $DPL_INCLUDE -o lulesh-viz.o lulesh-viz.cc"
BUILD_LULESH_UTIL_CC_COMMAND="icpx -fsycl -c -w -std=c++17 -DUSE_MPI=0 $CXX_FLAGS -I $DPL_INCLUDE -o lulesh-util.o lulesh-util.cc"
BUILD_LULESH_INIT_CC_COMMAND="icpx -fsycl -c -w -std=c++17 -DUSE_MPI=0 $CXX_FLAGS -I $DPL_INCLUDE -o lulesh-init.o lulesh-init.cc"

echo $BUILD_LULESH_CC_COMMPAND
$BUILD_LULESH_CC_COMMPAND

echo $BUILD_LULESH_COMM_CC_COMMAND
$BUILD_LULESH_COMM_CC_COMMAND

echo $BUILD_LULESH_VIZ_CC_COMMAND
$BUILD_LULESH_VIZ_CC_COMMAND

echo $BUILD_LULESH_UTIL_CC_COMMAND
$BUILD_LULESH_UTIL_CC_COMMAND

echo $BUILD_LULESH_VIZ_CC_COMMAND
$BUILD_LULESH_VIZ_CC_COMMAND

echo "linking lulesh"
LINK_LULESH_COMMAND="icpx -fsycl -w -std=c++17 lulesh.o lulesh-comm.o lulesh-viz.o lulesh-util.o lulesh-init.o -o lulesh -ltbb $LINK_FLAGS"

echo $LINK_LULESH_COMMAND
$LINK_LULESH_COMMAND
