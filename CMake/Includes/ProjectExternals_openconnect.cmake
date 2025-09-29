add_executable(openconnect::app IMPORTED)
set_property(TARGET openconnect::app PROPERTY IMPORTED_LOCATION ${CMAKE_PREFIX_PATH}/sbin/openconnect.exe)

add_library(openconnect::gmp SHARED IMPORTED)
set_property(TARGET openconnect::gmp PROPERTY IMPORTED_LOCATION ${CMAKE_PREFIX_PATH}/bin/libgmp-10.dll)
set_property(TARGET openconnect::gmp PROPERTY IMPORTED_IMPLIB ${CMAKE_PREFIX_PATH}/lib/libgmp.dll.a)

add_library(openconnect::gnutls SHARED IMPORTED)
set_property(TARGET openconnect::gnutls PROPERTY IMPORTED_LOCATION ${CMAKE_PREFIX_PATH}/bin/libgnutls-30.dll)
set_property(TARGET openconnect::gnutls PROPERTY IMPORTED_IMPLIB ${CMAKE_PREFIX_PATH}/lib/libgnutls.dll.a)

add_library(openconnect::hogweed SHARED IMPORTED)
set_property(TARGET openconnect::hogweed PROPERTY IMPORTED_LOCATION ${CMAKE_PREFIX_PATH}/bin/libhogweed-4.dll)
set_property(TARGET openconnect::hogweed PROPERTY IMPORTED_IMPLIB ${CMAKE_PREFIX_PATH}/lib/libhogweed.dll.a)

add_library(openconnect::nettle SHARED IMPORTED)
set_property(TARGET openconnect::nettle PROPERTY IMPORTED_LOCATION ${CMAKE_PREFIX_PATH}/bin/libnettle-6.dll)
set_property(TARGET openconnect::nettle PROPERTY IMPORTED_IMPLIB ${CMAKE_PREFIX_PATH}/lib/libnettle.dll.a)

add_library(openconnect::openconnect SHARED IMPORTED)
set_property(TARGET openconnect::openconnect PROPERTY IMPORTED_LOCATION ${CMAKE_PREFIX_PATH}/bin/libopenconnect-5.dll)
set_property(TARGET openconnect::openconnect PROPERTY IMPORTED_IMPLIB ${CMAKE_PREFIX_PATH}/lib/libopenconnect.dll.a)

add_library(openconnect::p11-kit SHARED IMPORTED)
set_property(TARGET openconnect::p11-kit PROPERTY IMPORTED_LOCATION ${CMAKE_PREFIX_PATH}/bin/libp11-kit-0.dll)
set_property(TARGET openconnect::p11-kit PROPERTY IMPORTED_IMPLIB ${CMAKE_PREFIX_PATH}/lib/libp11-kit.dll.a)

add_library(openconnect::stoken SHARED IMPORTED)
set_property(TARGET openconnect::stoken PROPERTY IMPORTED_LOCATION ${CMAKE_PREFIX_PATH}/bin/libstoken-1.dll)
set_property(TARGET openconnect::stoken PROPERTY IMPORTED_IMPLIB ${CMAKE_PREFIX_PATH}/lib/libstoken.dll.a)

add_library(openconnect::xml2 SHARED IMPORTED)
set_property(TARGET openconnect::xml2 PROPERTY IMPORTED_LOCATION ${CMAKE_PREFIX_PATH}/bin/libxml2-2.dll)
set_property(TARGET openconnect::xml2 PROPERTY IMPORTED_IMPLIB ${CMAKE_PREFIX_PATH}/lib/libxml2.dll.a)

add_library(openconnect::wintun SHARED IMPORTED)
set_property(TARGET openconnect::wintun PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/bin/wintun.dll)

install(
    FILES
        ${CMAKE_PREFIX_PATH}/bin/openconnect.exe
        ${CMAKE_BINARY_DIR}/bin/vpnc-script-win.js
    DESTINATION .
    COMPONENT App_Console
)
