Depends on TBB, found it via environment.
To build under Linux
```
CXX=`which clang++` cmake -DDPL_PATH=/mnt/c/Users/akonoval/src/oneDPL -DCMAKE_BUILD_TYPE=Debug -G Ninja ..
```

To build under Windows
```
cmake -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_COMPILER=icx -DDPL_PATH=C:/Users/akonoval/src/oneDPL ..
```
 Note that replacement of memory allocation functions is not supported for debug Windows runtime.
