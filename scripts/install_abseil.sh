#!/bin/bash

#!/bin/sh

set -e

ABSEIL_PATH="../deps/"

# Check to see if the current directory is the root of the project
if [ -f "LICENSE" ]; then
    ABSEIL_PATH="deps/"
fi

# Parse arguments
if [ $# -eq 1 ]; then
    ABSEIL_PATH=$1
else
    mkdir -p "$ABSEIL_PATH"
fi

# Convert into absolute path
ABSEIL_PATH=$(cd "$ABSEIL_PATH"; pwd)
ABSEIL_PATH="${ABSEIL_PATH}/abseil/"

echo "Installation path for Abseil: $ABSEIL_PATH"

CURRENT_DIR=$(pwd)

mkdir -p "${ABSEIL_PATH}"
git clone https://github.com/abseil/abseil-cpp.git
cd abseil-cpp/ && mkdir build && cd build
cmake -DABSL_BUILD_TESTING=ON -DABSL_USE_GOOGLETEST_HEAD=ON -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX="$ABSEIL_PATH" ..
cmake --build . --target install -j$(nproc)
cd "$CURRENT_DIR"
rm -rf abseil-cpp
