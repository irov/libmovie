#! /bin/bash

PROJECT=moview
CONFIGURATION=Debug
SOLUTION_DIR=../solutions/libmovie_xcode_$CONFIGURATION

pushd ..
mkdir -p $SOLUTION_DIR
pushd $SOLUTION_DIR
/Applications/CMake.app/Contents/bin/cmake -G "Xcode" "${PWD}/../.." -DCMAKE_CONFIGURATION_TYPES:STRING=$CONFIGURATION -DCMAKE_BUILD_TYPE:STRING=$CONFIGURATION
popd
popd
