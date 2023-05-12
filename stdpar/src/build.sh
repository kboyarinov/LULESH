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

FLAGS=""

if [ "$DEBUG" == "ON" ]
then
    echo "Debug mode"
    FLAGS+="-DSTDPAR_DEBUG"
fi

echo "building lulesh"

if [ "$STDPAR" == "ON" ]
then
    icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 -DSTDPAR -DUSE_USM_VECTOR $FLAGS -I /tmp/kboyarin/LULESH/generated_headers -I /tmp/kboyarin/oneDPL/include -o lulesh.o lulesh.cc
    icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 -DUSE_USM_VECTOR $FLAGS -I /tmp/kboyarin/oneDPL/include -o lulesh-comm.o lulesh-comm.cc
    icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 -DUSE_USM_VECTOR $FLAGS -I /tmp/kboyarin/oneDPL/include -o lulesh-viz.o lulesh-viz.cc
    icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 -DUSE_USM_VECTOR $FLAGS -I /tmp/kboyarin/oneDPL/include -o lulesh-util.o lulesh-util.cc
    icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 -DUSE_USM_VECTOR $FLAGS -I /tmp/kboyarin/oneDPL/include -o lulesh-init.o lulesh-init.cc
else
    icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 $FLAGS -I /tmp/kboyarin/oneDPL/include -o lulesh.o lulesh.cc
    icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 $FLAGS -I /tmp/kboyarin/oneDPL/include -o lulesh-comm.o lulesh-comm.cc
    icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 $FLAGS -I /tmp/kboyarin/oneDPL/include -o lulesh-viz.o lulesh-viz.cc
    icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 $FLAGS -I /tmp/kboyarin/oneDPL/include -o lulesh-util.o lulesh-util.cc
    icpx -fsycl -O3 -c -w -std=c++17 -DUSE_MPI=0 $FLAGS -I /tmp/kboyarin/oneDPL/include -o lulesh-init.o lulesh-init.cc
fi

echo "linking lulesh"
icpx -fsycl -O3 -w -std=c++17 lulesh.o lulesh-comm.o lulesh-viz.o lulesh-util.o lulesh-init.o -o lulesh -ltbb
