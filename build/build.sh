#!/bin/bash

# cmake .. -DCMAKE_TOOLCHAIN_FILE=../../platforms/armv5l/arm-linux-gnueabi.cmake -DCMAKE_BUILD_TYPE=Debug -DDISABLE_UT=OFF
cmake .. -DCMAKE_TOOLCHAIN_FILE=platforms/armv5l/arm-linux-gnueabi.cmake -DCMAKE_BUILD_TYPE=Debug -DDISABLE_UT=OFF
