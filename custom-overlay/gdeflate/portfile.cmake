# The GDeflate CPU codec for Android (sources/HAL/Android/HAL.GDeflate.cpp), from Microsoft's reference in
# microsoft/DirectStorage/GDeflate. That reference is two layers:
#  - the codec, NVIDIA's libdeflate fork, a git submodule of the reference pinned at LIBDEFLATE_COMMIT. Its own build
#    is a Makefile; the port builds it from its own CMakeLists.txt, with the reference's source list.
#  - the tile-stream framing DirectStorage writes (64 KB tiles behind an 8-byte header and a tile offset table):
#    TileStream.h and the two headers it includes are installed as they are. Its Compress/Decompress .cpp files
#    are not used: they start threads per call, and Decompress checks neither the output size against the header
#    nor the tile offsets against the input, and gives the last, shorter tile a full 64 KB of room in the output.
#
# Bumping DIRECTSTORAGE_COMMIT: take LIBDEFLATE_COMMIT from `git ls-tree <commit> GDeflate/3rdparty/libdeflate`.
vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
set(VCPKG_BUILD_TYPE release)

set(DIRECTSTORAGE_COMMIT 303a7d239e27c615281d6304ed13f01dc35061ef)
set(LIBDEFLATE_COMMIT 8ba9502fb30d2bf728592d121f0d402e40c8cb05)

vcpkg_download_distfile(LIBDEFLATE_ARCHIVE
    URLS "https://github.com/NVIDIA/libdeflate/archive/${LIBDEFLATE_COMMIT}.zip"
    FILENAME "nvidia-libdeflate-${LIBDEFLATE_COMMIT}.zip"
    SHA512 e5888177e68e936cc9c1c6d4c48875b2e8088a9bd73a7f7859ae9a337f5897660a462390efd4c6b677d8ce67026c682f870a2ed2f18b3cf2cb1580e19311939e
)
vcpkg_extract_source_archive(SOURCE_PATH
    ARCHIVE "${LIBDEFLATE_ARCHIVE}"
    SOURCE_BASE "libdeflate-${LIBDEFLATE_COMMIT}"
)

set(REFERENCE_URL "https://raw.githubusercontent.com/microsoft/DirectStorage/${DIRECTSTORAGE_COMMIT}/GDeflate/GDeflate")
vcpkg_download_distfile(TILESTREAM_H
    URLS "${REFERENCE_URL}/TileStream.h"
    FILENAME "gdeflate-TileStream-${DIRECTSTORAGE_COMMIT}.h"
    SHA512 e4c2a991fab4ab3a1b4e9edcf084418dde53d73befeaa894a7d03e614d3f90fd18aa3f99e3d262a9da4b344cb6caacb113cdf1ecf8cb4c8c7e950938ea173cbd
)
vcpkg_download_distfile(UTILS_H
    URLS "${REFERENCE_URL}/Utils.h"
    FILENAME "gdeflate-Utils-${DIRECTSTORAGE_COMMIT}.h"
    SHA512 b908d343f93b512ad99ee144fcc3c73f809c28e4ff93d7629ccb9a82cdc95c73680e1cbb16de8aee37fa221c88afb493d1449ee4da28076a2b18bb1b8acb0b63
)
vcpkg_download_distfile(CONFIG_H
    URLS "${REFERENCE_URL}/config.h"
    FILENAME "gdeflate-config-${DIRECTSTORAGE_COMMIT}.h"
    SHA512 713ed1a41729819c9d70db4e78042eb28c3b5c0a6516d0aca24791e070ea85fbaad292ce009a4bfa06a88267e8569812d51f199832933c45edcf768c9d0cdadb
)
vcpkg_download_distfile(REFERENCE_LICENSE
    URLS "${REFERENCE_URL}/LICENSE"
    FILENAME "gdeflate-LICENSE-${DIRECTSTORAGE_COMMIT}.txt"
    SHA512 b99f42cca2b8237641ff3dafd5070b44da1474604538c6cbfc4137aa631f5e4838c954f4e6039a007a5aea447537aae168e99fe10a2b32aec36a29dea95c0ef5
)

file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}/vcpkg")

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}/vcpkg"
    OPTIONS
        -DLIBDEFLATE_SOURCE_DIR=${SOURCE_PATH}
)
vcpkg_cmake_install()

set(INCLUDE_DIR "${CURRENT_PACKAGES_DIR}/include/gdeflate")
file(INSTALL "${TILESTREAM_H}" DESTINATION "${INCLUDE_DIR}" RENAME "TileStream.h")
file(INSTALL "${UTILS_H}" DESTINATION "${INCLUDE_DIR}" RENAME "Utils.h")
file(INSTALL "${CONFIG_H}" DESTINATION "${INCLUDE_DIR}" RENAME "config.h")

# libdeflate's COPYING holds both its MIT text and NVIDIA's Apache-2.0 notice for the GDeflate files; the reference's
# LICENSE file is the Apache-2.0 text for TileStream.h, Utils.h and config.h.
vcpkg_install_copyright(FILE_LIST
    "${SOURCE_PATH}/COPYING"
    "${REFERENCE_LICENSE}"
)

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
