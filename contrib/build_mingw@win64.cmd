@echo off
REM
REM Sample build script & release package preparation
REM
REM It should be used only as illustration how to build application
REM and create an installer package
REM
REM (c) 2016-2021, Lubomir Carik
REM

echo "======================================================================="
echo " Preparing environment..."
echo "======================================================================="
REM look for "Qt 5.15.2 for Desktop (MinGW 8.1.0 64 bit)" StartMenu item
REM and check 'qtenv2.bat'
echo Setting up environment for Qt usage...
set PATH=C:\Dev\Qt\5.15.2\mingw81_64\bin\;%PATH%

echo Setting up environment for 'mingw64' usage...
set PATH=c:\Dev\Qt\Tools\mingw810_64\bin\;%PATH%

echo Setting up environment for CMake usage...
set PATH="C:\Program Files\CMake\bin";%PATH%

echo Setting up environment for 7z usage...
set PATH="C:\Program Files\7-Zip\";%PATH%

echo Setting up environment for 'Ninja' usage...
set PATH="C:\Dev\";%PATH%

echo Setting up environment for 'clang' usage...
set PATH="C:\Dev\LLVM\bin\";%PATH%

echo Setting up environment for 'wix' toolset usage...
set PATH="C:\Program Files (x86)\WiX Toolset v3.11\bin";%PATH%
set WIX="C:\Program Files (x86)\WiX Toolset v3.11\"
set CPACK_WIX_ROOT="C:\Program Files (x86)\WiX Toolset v3.11\"

echo "======================================================================="
echo " Preparing sandbox..."
echo "======================================================================="
rd /s /q build-release64
md build-release64

echo "======================================================================="
echo " Generating project..."
echo "======================================================================="
cd build-release64
cmake -G "MinGW Makefiles" ^
    -DCMAKE_BUILD_TYPE=Release ^
    ..\..\

echo "======================================================================="
echo " Compiling..."
echo "======================================================================="
mingw32-make -j5

REM echo "======================================================================="
REM echo " LC: Bundling... (dynamic Qt only)"
REM echo "======================================================================="
REM rd /s /q out
REM md out
REM windeployqt ^
REM     src\openconnect-gui.exe ^
REM     --verbose 1 ^
REM     --compiler-runtime ^
REM     --release ^
REM     --force ^
REM     --no-webkit2 ^
REM     --no-quick-import ^
REM     --no-translations

echo "======================================================================="
echo " Packaging..."
echo "======================================================================="
cmake .
mingw32-make package VERBOSE=1
REM mingw32-make package_source VERBOSE=1

move /Y *.exe ..
move /Y *.exe.sha512 ..

cd ..
