# Replacement build for NVIDIA's libdeflate fork (its own build is a Makefile); see portfile.cmake.
# The source list is the one the reference builds (GDeflate/3rdparty/libdeflate.cmake), with the ARM feature detection
# next to the x86 one: each compiles to nothing on the other architecture. The GDeflate files are not self-contained:
# gdeflate_compress.c calls libdeflate_deflate_compress_bound() from deflate_compress.c.
cmake_minimum_required(VERSION 3.22)
project(gdeflate LANGUAGES C)

include(GNUInstallDirs)

set(LIB "${LIBDEFLATE_SOURCE_DIR}/lib")

add_library(gdeflate STATIC
    ${LIB}/adler32.c
    ${LIB}/crc32.c
    ${LIB}/deflate_compress.c
    ${LIB}/deflate_decompress.c
    ${LIB}/gdeflate_compress.c
    ${LIB}/gdeflate_decompress.c
    ${LIB}/gzip_compress.c
    ${LIB}/gzip_decompress.c
    ${LIB}/utils.c
    ${LIB}/zlib_compress.c
    ${LIB}/zlib_decompress.c
    ${LIB}/arm/cpu_features.c
    ${LIB}/x86/cpu_features.c
)

target_include_directories(gdeflate PRIVATE "${LIBDEFLATE_SOURCE_DIR}")

set_target_properties(gdeflate PROPERTIES
    C_STANDARD 99
    C_EXTENSIONS YES
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS gdeflate ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES "${LIBDEFLATE_SOURCE_DIR}/libdeflate.h" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/gdeflate")
