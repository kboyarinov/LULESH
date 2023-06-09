#!/bin/bash

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

GENERATED_HEADERS="/tmp/kboyarin/LULESH/generated_headers"
DPL_INCLUDE="/tmp/kboyarin/oneDPL/include"

CXX_FLAGS="-DLULESH_STDPAR_POLICY=$LULESH_STDPAR_POLICY"
LINK_FLAGS=""

if [ "$STDPAR" == "ON" ]
then
    CXX_FLAGS+=" -DSTDPAR -I $GENERATED_HEADERS"
    LINK_FLAGS+=" -loverusm"
fi

if [ "$DEBUG" == "ON" ]
then
    CXX_FLAGS+=" -DSTDPAR_DEBUG"
fi

if [ "$MEASURE_EACH" == "ON" ]
then
    CXX_FLAGS+=" -DMEASURE_EACH_ALGORITHM"
fi

if [ "$USE_ONEDPL" == "ON" ]
then
    CXX_FLAGS+=" -DUSE_ONEDPL"
fi

echo "building lulesh: GPU STDPAR = $STDPAR LULESH_STDPAR_POLICY = $LULESH_STDPAR_POLICY DEBUG = $DEBUG MEASURE EACH = $MEASURE_EACH"

icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 $CXX_FLAGS -I $DPL_INCLUDE -o lulesh.o lulesh.cc
icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 -I $DPL_INCLUDE -o lulesh-comm.o lulesh-comm.cc
icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 -I $DPL_INCLUDE -o lulesh-viz.o lulesh-viz.cc
icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 -I $DPL_INCLUDE -o lulesh-util.o lulesh-util.cc
icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 -I $DPL_INCLUDE -o lulesh-init.o lulesh-init.cc

echo "linking lulesh"
icpx -fsycl -O3 -w -std=c++17 lulesh.o lulesh-comm.o lulesh-viz.o lulesh-util.o lulesh-init.o -o lulesh -ltbb $LINK_FLAGS
