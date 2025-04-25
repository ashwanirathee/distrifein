#!/bin/bash

set -x

rm -r build/
mkdir build
cd build
cmake ..
make

set +x