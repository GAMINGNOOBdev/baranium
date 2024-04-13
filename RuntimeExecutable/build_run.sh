#!/bin/bash

# delete binary directories
rm -rf build
rm -rf bin

# create and enter build directory
mkdir build
cd build

# build and go back to root
cmake ..
make
cd ..

# create binary directory and copy the final binary

mkdir bin
cp build/bgs bin

# RUN
./bin/bgs ../Compiler/test/CustomScript01.bxs