#!/bin/sh

set -e

ABSEIL_PATH="../deps/"

# Parse arguments
if [ $# -eq 1 ]; then
    ABSEIL_PATH=$1
else
    mkdir -p ../deps
fi

# Check if directory exists
#if [ ! -d "$ABSEIL_PATH" ]; then
#    echo "Installation directory $ABSEIL_PATH DOES NOT exists!"
#    exit 1
#fi

# Convert into absolute path
ABSEIL_PATH=$(cd "$ABSEIL_PATH"; pwd)
ABSEIL_PATH="${ABSEIL_PATH}/abseil/"

echo "Installation path for Abseil: $ABSEIL_PATH"

CURRENT_DIR=$(pwd)

mkdir -p "${ABSEIL_PATH}"
# git clone https://github.com/abseil/abseil-cpp.git
wget https://github.com/abseil/abseil-cpp/archive/refs/tags/20240116.1.zip
unzip 20240116.1.zip
cd abseil-cpp-20240116.1/ && mkdir build && cd build
cmake                                     \
	-DBUILD_TESTING=OFF                   \
	-DABSL_BUILD_TESTING=OFF              \
	-DABSL_USE_GOOGLETEST_HEAD=OFF        \
	-DCMAKE_CXX_STANDARD=17               \
	-DCMAKE_INSTALL_PREFIX="$ABSEIL_PATH" \
	..
cmake --build . --target install -j"$(nproc)"
cd "$CURRENT_DIR"
rm -rf abseil-cpp
