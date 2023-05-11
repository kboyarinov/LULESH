#!/bin/bash

if [ -z "$STDPAR" ]
then
    STDPAR = "OFF"
fi

if [ "$STDPAR" != "ON" && "$STDPAR" != "OFF" ]
then
    echo "Incorrect value of STDPAR environment variable"
    exit 1
fi

echo "building lulesh"

if [ "$STDPAR" == "ON" ]
then
    icpx -fsycl -w -std=c++17 -DUSE_MPI=0 -DSTDPAR -DUSE_USM_VECTOR -I /tmp/kboyarin/LULESH/generated_headers -I /tmp/kboyarin/oneDPL/include -o lulesh.o lulesh.cc
    icpx -fsycl -w -std=c++17 -DUSE_MPI=0 -DUSE_USM_VECTOR -I /tmp/kboyarin/oneDPL/include -o lulesh-comm.o lulesh-comm.cc
    icpx -fsycl -w -std=c++17 -DUSE_MPI=0 -DUSE_USM_VECTOR -I /tmp/kboyarin/oneDPL/include -o lulesh-viz.o lulesh-viz.cc
    icpx -fsycl -w -std=c++17 -DUSE_MPI=0 -DUSE_USM_VECTOR -I /tmp/kboyarin/oneDPL/include -o lulesh-util.o lulesh-util.cc
    icpx -fsycl -w -std=c++17 -DUSE_MPI=0 -DUSE_USM_VECTOR -I /tmp/kboyarin/oneDPL/include -o lulesh-init.o lulesh-init.cc
else
    ipcx -fsycl -w -std=c++17 -DUSE_MPI=0 -I /tmp/kboyarin/oneDPL/include -o lulesh.o lulesh.cc
    ipcx -fsycl -w -std=c++17 -DUSE_MPI=0 -I /tmp/kboyarin/oneDPL/include -o lulesh-comm.o lulesh-comm.cc
    ipcx -fsycl -w -std=c++17 -DUSE_MPI=0 -I /tmp/kboyarin/oneDPL/include -o lulesh-viz.o lulesh-viz.cc
    ipcx -fsycl -w -std=c++17 -DUSE_MPI=0 -I /tmp/kboyarin/oneDPL/include -o lulesh-util.o lulesh-util.cc
    ipcx -fsycl -w -std=c++17 -DUSE_MPI=0 -I /tmp/kboyarin/oneDPL/include -o lulesh-init.o lulesh-init.cc
fi

echo "linking lulesh"
icpx -fsycl -w -std=c++17 lulesh.o lulesh-comm.o lulesh-viz.o lulesh-util.o lulesh-init.o -o lulesh
