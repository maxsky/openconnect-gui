#
# Sample build script & release package preparation for OpenConnect-GUI project
# with MINGW64 on MSYS2 toolchain
#
# It should be used only as illustration how to build application
# and create an installer package
#
# (c) 2016-2021, Lubomir Carik
#

echo "Starting under $MSYSTEM build environment..."

if [ "$1" == "--head" ]; then
    export OC_TAG=master
else
    export OC_TAG=v9.12
fi

pacman --needed --noconfirm -S \
    mingw-w64-x86_64-cmake \
    mingw-w64-x86_64-nsis \
    mingw-w64-x86_64-qt5

echo "======================================================================="
echo " Preparing sandbox..."
echo "======================================================================="
[ -d build-ocg-$MSYSTEM ] || mkdir -pv build-ocg-$MSYSTEM
cd build-ocg-$MSYSTEM

echo "======================================================================="
echo " Generating project..."
echo "======================================================================="
cmake -G "MinGW Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -Dopenconnect-TAG=${OC_TAG} \
    ../..

echo "======================================================================="
echo " Compiling..."
echo "======================================================================="
CORES=$(getconf _NPROCESSORS_ONLN)
cmake --build . -- -j${CORES}

# echo "======================================================================="
# echo " LC: Bundling... (dynamic Qt only)"
# echo "======================================================================="
# rd /s /q out
# md out
# windeployqt ^
#   src\openconnect-gui.exe ^
#   --verbose 1 ^
#   --compiler-runtime ^
#   --release ^
#   --force ^
#   --no-webkit2 ^
#   --no-quick-import ^
#   --no-translations

echo "======================================================================="
echo " Packaging..."
echo "======================================================================="
cmake .
cmake --build . --target package -- VERBOSE=1
# make package_source VERBOSE=1

mv -vf *.exe ../..
mv -vf *.exe.sha512 ../..

cd ..
