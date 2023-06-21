icpx -fsycl -c -w -std=c++17 -DUSE_MPI=0 -DSTDPAR -DUSE_USM_VECTOR -I C:\git\stdpar\generated_headers -I C:\git\oneDPL\include -o lulesh.o lulesh.cc
icpx -fsycl -c -w -std=c++17 -DUSE_MPI=0 -DSTDPAR -DUSE_USM_VECTOR -I C:\git\stdpar\generated_headers -I C:\git\oneDPL\include -o lulesh-comm.o lulesh-comm.cc
icpx -fsycl -c -w -std=c++17 -DUSE_MPI=0 -DSTDPAR -DUSE_USM_VECTOR -I C:\git\stdpar\generated_headers -I C:\git\oneDPL\include -o lulesh-viz.o lulesh-viz.cc
icpx -fsycl -c -w -std=c++17 -DUSE_MPI=0 -DSTDPAR -DUSE_USM_VECTOR -I C:\git\stdpar\generated_headers -I C:\git\oneDPL\include -o lulesh-util.o lulesh-util.cc
icpx -fsycl -c -w -std=c++17 -DUSE_MPI=0 -DSTDPAR -DUSE_USM_VECTOR -I C:\git\stdpar\generated_headers -I C:\git\oneDPL\include -o lulesh-init.o lulesh-init.cc

icpx -fsycl -w -std=c++17 -DUSE_MPI=0 -DSTDPAR -DUSE_USM_VECTOR lulesh.o lulesh-comm.o lulesh-viz.o lulesh-util.o lulesh-init.o -o lulesh.exe
