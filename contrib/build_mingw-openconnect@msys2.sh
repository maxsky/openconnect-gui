#
# Sample script to checkout & build 'openconnect' project
# with MINGW64 on MSYS2 toolchain
#
# It should be used only as illustration how to build application
# and create an installer package
#
# (c) 2018-2021, Lubomir Carik
#

if [ "$MSYSTEM" == "MINGW64" ]; then
    export BUILD_ARCH=x86_64
    export MINGW_PREFIX=/mingw64
elif [ "$MSYSTEM" == "MINGW32" ]; then
    export BUILD_ARCH=i686
    export MINGW_PREFIX=/mingw32
else
    echo "Unknown MSYS2 build environment..."
    exit -1
fi
echo "Starting under $MSYSTEM build environment..."

if [ "$1" == "--head" ]; then
    export OC_TAG=master
    export STOKEN_TAG=master
else
    export OC_TAG=v9.12
    export STOKEN_TAG=v0.92
fi

export OC_URL=https://gitlab.com/openconnect/openconnect.git
export STOKEN_URL=https://github.com/cernekee/stoken

pacman --needed --noconfirm -S \
    git \
    p7zip \
    base-devel \
    autotools \
    mingw-w64-x86_64-toolchain \
    mingw-w64-x86_64-jq \
    mingw-w64-${BUILD_ARCH}-gcc \
    mingw-w64-${BUILD_ARCH}-make \
    mingw-w64-${BUILD_ARCH}-gnutls \
    mingw-w64-${BUILD_ARCH}-libidn2 \
    mingw-w64-${BUILD_ARCH}-libunistring \
    mingw-w64-${BUILD_ARCH}-nettle \
    mingw-w64-${BUILD_ARCH}-gmp \
    mingw-w64-${BUILD_ARCH}-p11-kit \
    mingw-w64-${BUILD_ARCH}-zlib \
    mingw-w64-${BUILD_ARCH}-libxml2 \
    mingw-w64-${BUILD_ARCH}-zlib \
    mingw-w64-${BUILD_ARCH}-lz4 \
    mingw-w64-${BUILD_ARCH}-libproxy

#openconnect compilation is broken on recent versions (>=2.12) of libxml2 because of header reorg
#(see https://gitlab.com/openconnect/openconnect/-/issues/685)
#
#use latest 2.11 version until openconnect is fixed
#TODO remove the following line after bumping OC_TAG (hopefully to v9.13)
pacman --needed --noconfirm -U https://repo.msys2.org/mingw/mingw64/mingw-w64-x86_64-libxml2-2.11.6-1-any.pkg.tar.zst


[ -d build-oc-$MSYSTEM ] || mkdir build-oc-$MSYSTEM
cd build-oc-$MSYSTEM

#CORES=$(grep -c ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu || echo "$NUMBER_OF_PROCESSORS")
CORES=$(getconf _NPROCESSORS_ONLN)

[ -d stoken ] || git clone ${STOKEN_URL}
cd stoken
git clean -fdx
git reset --hard
git checkout -f -b ${STOKEN_TAG} ${STOKEN_TAG}
./autogen.sh
[ -d build-${BUILD_ARCH} ] || mkdir build-${BUILD_ARCH}
cd build-${BUILD_ARCH}
../configure --disable-dependency-tracking --without-tomcrypt --without-gtk
mingw32-make -j${CORES}
mingw32-make install
cd ../../

[ -d openconnect ] || git clone ${OC_URL}
cd openconnect
git clean -fdx
git reset --hard
git checkout -f -b ${OC_TAG} ${OC_TAG}
echo "hash:"
git rev-parse --short HEAD | tee ../openconnect-${OC_TAG}_$MSYSTEM.hash
./autogen.sh
[ -d build-${BUILD_ARCH} ] || mkdir build-${BUILD_ARCH}
cd build-${BUILD_ARCH}

#disable libproxy since libproxy >= 0.5 has a lot of dependencies that expand the attack surface
#see https://gitlab.com/openconnect/openconnect-gui/-/merge_requests/259#note_1713843295
#also libproxy is probably not used by openconnect-gui
../configure --disable-dependency-tracking --with-gnutls --without-openssl --without-libpskc --without-libproxy --with-vpnc-script=vpnc-script-win.js
mingw32-make -j${CORES}
cd ../../


#
# Sample script to create a package from build 'openconnect' project
# incl. all dependencies (hardcoded paths!)
#

rm -rf pkg
mkdir -p pkg/nsis && cd pkg/nsis
cp ${MINGW_PREFIX}/bin/libffi-8.dll .
cp ${MINGW_PREFIX}/bin/libgcc_*-1.dll .
cp ${MINGW_PREFIX}/bin/libgmp-10.dll .
cp ${MINGW_PREFIX}/bin/libgnutls-30.dll .
cp ${MINGW_PREFIX}/bin/libhogweed-6.dll .
cp ${MINGW_PREFIX}/bin/libintl-8.dll .
cp ${MINGW_PREFIX}/bin/libnettle-8.dll .
cp ${MINGW_PREFIX}/bin/libp11-kit-0.dll .
cp ${MINGW_PREFIX}/bin/libtasn1-6.dll .
cp ${MINGW_PREFIX}/bin/libwinpthread-1.dll .
cp ${MINGW_PREFIX}/bin/libxml2-2.dll .
cp ${MINGW_PREFIX}/bin/zlib1.dll .
cp ${MINGW_PREFIX}/bin/libstoken-1.dll .
cp ${MINGW_PREFIX}/bin/liblz4.dll .
cp ${MINGW_PREFIX}/bin/libiconv-2.dll .
cp ${MINGW_PREFIX}/bin/libunistring-5.dll .
cp ${MINGW_PREFIX}/bin/libidn2-0.dll .
cp ${MINGW_PREFIX}/bin/liblzma-5.dll .
cp ${MINGW_PREFIX}/bin/libbrotlicommon.dll .
cp ${MINGW_PREFIX}/bin/libbrotlidec.dll .
cp ${MINGW_PREFIX}/bin/libzstd.dll .
cp ${MINGW_PREFIX}/bin/libbrotlienc.dll .
cp ../../openconnect/build-${BUILD_ARCH}/.libs/libopenconnect-5.dll .
cp ../../openconnect/build-${BUILD_ARCH}/.libs/wintun.dll .
cp ../../openconnect/build-${BUILD_ARCH}/.libs/openconnect.exe .
curl    -o vpnc-script-win.js https://gitlab.com/openconnect/vpnc-scripts/-/raw/master/vpnc-script-win.js
cd ../../

mkdir -p pkg/lib && cd pkg/lib
cp ${MINGW_PREFIX}/lib/libgmp.dll.a .
cp ${MINGW_PREFIX}/lib/libgnutls.dll.a .
cp ${MINGW_PREFIX}/lib/libhogweed.dll.a .
cp ${MINGW_PREFIX}/lib/libnettle.dll.a .
cp ${MINGW_PREFIX}/lib/libp11-kit.dll.a .
cp ${MINGW_PREFIX}/lib/libxml2.dll.a .
cp ${MINGW_PREFIX}/lib/libz.dll.a .
cp ${MINGW_PREFIX}/lib/libstoken.dll.a .
cp ${MINGW_PREFIX}/lib/liblz4.dll.a .
cp ${MINGW_PREFIX}/lib/libiconv.dll.a .
cp ${MINGW_PREFIX}/lib/libunistring.dll.a .
cp ${MINGW_PREFIX}/lib/libidn2.dll.a .
cp ${MINGW_PREFIX}/lib/liblzma.dll.a .
cp ../../openconnect/build-${BUILD_ARCH}/.libs/libopenconnect.dll.a .
cd ../../

mkdir -p pkg/lib/pkgconfig && cd pkg/lib/pkgconfig
cp ${MINGW_PREFIX}/lib/pkgconfig/gnutls.pc .
cp ${MINGW_PREFIX}/lib/pkgconfig/hogweed.pc .
cp ${MINGW_PREFIX}/lib/pkgconfig/libxml-2.0.pc .
cp ${MINGW_PREFIX}/lib/pkgconfig/nettle.pc .
cp ${MINGW_PREFIX}/lib/pkgconfig/zlib.pc .
cp ${MINGW_PREFIX}/lib/pkgconfig/stoken.pc .
cp ../../../openconnect/build-${BUILD_ARCH}/openconnect.pc .
cd ../../../

mkdir -p pkg/include && cd pkg/include
cp -R ${MINGW_PREFIX}/include/gnutls/ .
cp -R ${MINGW_PREFIX}/include/libxml2/ .
cp -R ${MINGW_PREFIX}/include/nettle/ .
cp -R ${MINGW_PREFIX}/include/p11-kit-1/p11-kit/ .
cp ${MINGW_PREFIX}/include/gmp.h .
cp ${MINGW_PREFIX}/include/zconf.h .
cp ${MINGW_PREFIX}/include/zlib.h .
cp ${MINGW_PREFIX}/include/stoken.h .
cp ../../openconnect/openconnect.h .
cd ../../

export MINGW_PREFIX=

cd pkg/nsis
7za a -tzip -mx=9 -sdel ../../openconnect-${OC_TAG}_$MSYSTEM.zip *
cd ../
rmdir -v nsis
7za a -tzip -mx=9 -sdel ../openconnect-devel-${OC_TAG}_$MSYSTEM.zip *
cd ../
rmdir -v pkg


#cd stoken/build-${BUILD_ARCH}
#sudo $MSYSTEM-make uninstall

echo "List of system-wide used packages versions:" \
    > openconnect-${OC_TAG}_$MSYSTEM.txt
echo "openconnect-${OC_TAG}" \
    >> openconnect-${OC_TAG}_$MSYSTEM.txt
echo "stoken-${STOKEN_TAG}" \
    >> openconnect-${OC_TAG}_$MSYSTEM.txt
pacman -Q \
    mingw-w64-${BUILD_ARCH}-gnutls \
    mingw-w64-${BUILD_ARCH}-libidn2 \
    mingw-w64-${BUILD_ARCH}-libunistring \
    mingw-w64-${BUILD_ARCH}-nettle \
    mingw-w64-${BUILD_ARCH}-gmp \
    mingw-w64-${BUILD_ARCH}-p11-kit \
    mingw-w64-${BUILD_ARCH}-libxml2 \
    mingw-w64-${BUILD_ARCH}-zlib \
    mingw-w64-${BUILD_ARCH}-lz4 \
    mingw-w64-${BUILD_ARCH}-libproxy \
    >> openconnect-${OC_TAG}_$MSYSTEM.txt

sha512sum.exe openconnect-${OC_TAG}_$MSYSTEM.zip > openconnect-${OC_TAG}_$MSYSTEM.zip.sha512
sha512sum.exe openconnect-devel-${OC_TAG}_$MSYSTEM.zip > openconnect-devel-${OC_TAG}_$MSYSTEM.zip.sha512

mv -vu openconnect-*.zip openconnect-*.txt openconnect-*.zip.sha512 openconnect-${OC_TAG}_$MSYSTEM.hash ../../external
