if(MINGW)
    if(NOT CMAKE_CROSSCOMPILING)
        set(openconnect-TAG "v9.12" CACHE STRING "Please specify openconnect version")
        set(openconnect-TAG_CHOICES "v9.12" "v8.10" "v7.08" "master")
        set_property(CACHE openconnect-TAG PROPERTY STRINGS ${openconnect-TAG_CHOICES})
        if(NOT openconnect-TAG IN_LIST openconnect-TAG_CHOICES)
            message(FATAL_ERROR "Specify 'openconnect-TAG'. Must be one of ${openconnect-TAG_CHOICES}")
        endif()
    endif()

    # source: http://build.openvpn.net/downloads/releases/
    set(tap-driver-os "Win10" CACHE STRING "Please specify tap-driver target OS")
    set(tap-driver-os_CHOICES "Win10" "Win7")
    set_property(CACHE tap-driver-os PROPERTY STRINGS ${tap-driver-os_CHOICES})
    if(NOT tap-driver-os IN_LIST tap-driver-os_CHOICES)
        message(FATAL_ERROR "Specify 'tap-driver-os'. Must be one of ${tap-driver-os_CHOICES}")
    endif()
    set(tap-driver-TAG 9.24.2)
endif()

set(vpnc-scripts-TAG master)
set(qt-solutions-TAG master)

if(CMAKE_CROSSCOMPILING AND MINGW)
    # Fedora mingw32/mingw64
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CMAKE_CROSS_COMMAND mingw64-cmake)
    else()
        set(CMAKE_CROSS_COMMAND mingw32-cmake)
    endif()
else()
    # Windows mingw32 & macOS & native GNU/Linux
    set(CMAKE_CROSS_COMMAND ${CMAKE_COMMAND})
endif()
message(STATUS "Using '${CMAKE_CROSS_COMMAND}' as CMake...")


include(ExternalProject)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/external/include)

if(NOT spdlog_FOUND)
    message(STATUS "Using local spdlog build")
    set(spdlog-TAG v1.8.5)
    include(ProjectExternals_spdlog)
endif()

include(ProjectExternals_qt-solutions)
if(MINGW)
    if (NOT CMAKE_CROSSCOMPILING)
        include(ProjectExternals_openconnect)
    endif()
    include(ProjectExternals_vpnc-scripts-win)
    include(ProjectExternals_tap-windows)
endif()

