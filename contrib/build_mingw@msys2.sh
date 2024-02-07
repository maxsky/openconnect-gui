#
# Sample build script & release package preparation for OpenConnect-GUI project
# with MINGW64 on MSYS2 toolchain
#
# It should be used only as illustration how to build application
# and create an installer package
#
# (c) 2016-2021, Lubomir Carik
#

BUILD_TYPE="${BUILD_TYPE:-Debug}"
BUILD_DIR="${BUILD_DIR:-build-$MSYSTEM}"
ROOT_DIR=$(pwd)

echo "Starting under $MSYSTEM build environment..."

if [ "$1" == "--head" ]; then
    export OC_TAG=master
else
    export OC_TAG=v9.12
fi

if [ -z "$QT5" ];then
    pacman --needed --noconfirm -S \
        mingw-w64-x86_64-cmake \
        mingw-w64-x86_64-nsis \
        mingw-w64-x86_64-qt5
fi

echo "======================================================================="
echo " Preparing sandbox..."
echo "======================================================================="
[ -d "${BUILD_DIR}" ] || mkdir "${BUILD_DIR}"
cd "${BUILD_DIR}"

set -e

echo "======================================================================="
echo " Generating project..."
echo "======================================================================="
cmake -G "MinGW Makefiles" \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -Dopenconnect-TAG=${OC_TAG} \
    -S .. -B .

echo "======================================================================="
echo " Compiling..."
echo "======================================================================="
CORES=$(getconf _NPROCESSORS_ONLN)
cmake --build . --config "$BUILD_TYPE" --target package -- -j${CORES}

cd ..
