#!/usr/bin/env bash

if [ ! -d "build" ]; then
    mkdir build
fi
cd build
cmake ..
cmake --build . --target GenerativeArt

