#!/bin/bash

make clean
make
./bin/bgsc -d -o test/CustomScript01.bxs test/CustomScript01.bgs test/CustomScript02.bgs -I test/includes.txt