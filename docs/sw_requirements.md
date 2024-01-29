### Software requirements
This chapter summarize actual development environment. Newer or older versions may work - feel free tweak main CMakeLists.txt and dependencies as you need.

#### Windows 10
This is target platform for the product.

- [Qt 5 (incl. mingw32) & QtCreator 4+](http://download.qt.io/official_releases/online_installers/qt-unified-windows-x86-online.exe) and choose:
    - 5.12.0/MinGW 7.3.0 64-bit
    - Tools/MinGW 7.3.0 64-bit
    - Tools/QtCreator component
    - Tools/QtCreator CDB Debugger support
- [NSIS 3.0+](https://sourceforge.net/projects/nsis)
- [CMake 3.12+](https://cmake.org/)
- [Git 2.30+](https://git-scm.com/) incl. Git LFS

##### Externals

These projects will be downloaded automatically by CMake.

- [OpenConnect for MinGW32/MinGW64](https://github.com/horar/openconnect/releases)
- [TAP-windows](https://openvpn.net/index.php/open-source/downloads.html)
- [spdlog](https://github.com/gabime/spdlog)
- [qt-solutions](https://github.com/qtproject/qt-solutions.git)


#### GNU/Linux
##### Fedora (MinGW32)
Just try install following packages:

    sudo dnf install -y git git-lfs
    sudo dnf install -y cmake
    sudo dnf install -y mingw32-qt5-qtbase
    sudo dnf install -y mingw32-nsis

##### Fedora
The minimum of required packages to build this GUI should be installed by following commands:

    sudo dnf install -y git gitflow
    sudo dnf install -y cmake
    sudo dnf install -y qt5-qtbase-devel gcc-c++
    sudo dnf install -y gnutls-devel openconnect-devel
    sudo dnf install -y spdlog-devel

After the binary is generated provide it with networking capabilities to be
able to use it, as follows:

    sudo setcap cap_net_admin=ep openconnect-gui

#### MacOS
- MacOS 10.13+
- Xcode 10.0+ incl. command line tools
- [Qt online installer](http://download.qt.io/official_releases/online_installers/qt-unified-mac-x64-online.dmg) and choose:
    - 5.12.0/"OS X" component
    - Tools/QtCreator component
