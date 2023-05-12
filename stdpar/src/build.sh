#!/bin/bash

if [ -z "$STDPAR" ]
then
    STDPAR = "OFF"
fi

if [ "$STDPAR" != "ON" ] && [ "$STDPAR" != "OFF" ]
then
    echo "Incorrect value of STDPAR environment variable"
    exit 1
fi

GENERATED_HEADERS="/home/sdp/kboyarin/LULESH/generated_headers"
DPL_INCLUDE="/home/sdp/kboyarin/oneDPL/include"

echo "building lulesh"

if [ "$STDPAR" == "ON" ]
then
    icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 -DSTDPAR -DUSE_USM_VECTOR -I $GENERATED_HEADERS -I $DPL_INCLUDE -o lulesh.o lulesh.cc
    icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 -DUSE_USM_VECTOR -I $DPL_INCLUDE -o lulesh-comm.o lulesh-comm.cc
    icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 -DUSE_USM_VECTOR -I $DPL_INCLUDE -o lulesh-viz.o lulesh-viz.cc
    icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 -DUSE_USM_VECTOR -I $DPL_INCLUDE -o lulesh-util.o lulesh-util.cc
    icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 -DUSE_USM_VECTOR -I $DPL_INCLUDE -o lulesh-init.o lulesh-init.cc
else
    icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 -I $DPL_INCLUDE -o lulesh.o lulesh.cc
    icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 -I $DPL_INCLUDE -o lulesh-comm.o lulesh-comm.cc
    icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 -I $DPL_INCLUDE -o lulesh-viz.o lulesh-viz.cc
    icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 -I $DPL_INCLUDE -o lulesh-util.o lulesh-util.cc
    icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 -I $DPL_INCLUDE -o lulesh-init.o lulesh-init.cc
fi

echo "linking lulesh"
icpx -fsycl -O3 -w -std=c++17 lulesh.o lulesh-comm.o lulesh-viz.o lulesh-util.o lulesh-init.o -o lulesh -ltbb
