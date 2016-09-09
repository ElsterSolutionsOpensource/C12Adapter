#!/usr/bin/env bash
# Building C12Adapter libraries and examples
BUILD_PATH=`dirname $0`/build
if [ ! -d "$BUILD_PATH" ]; then
   mkdir "$BUILD_PATH"
fi
cd "$BUILD_PATH"
cmake ..
cmake --build . --config Debug
