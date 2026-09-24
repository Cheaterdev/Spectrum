# Skribidi's own CMake can't be used as-is: it FetchContents harfbuzz,
# SheenBidi, libunibreak and budouxc over the network (forbidden in vcpkg's
# sandboxed build) and hard-sets CMAKE_MSVC_RUNTIME_LIBRARY to the static CRT,
# which would not link into Spectrum's /MD build. The port builds from its own
# CMakeLists.txt instead.
#
# SheenBidi, libunibreak and budouxc are compiled into skribidi.lib from the
# exact commits Skribidi's CMake pins, not taken from vcpkg: the baseline's
# sheenbidi is 3.0.0 and libunibreak 7.0, both major versions past what
# Skribidi is written and tested against. harfbuzz does come from vcpkg; its
# API is backward compatible and it is by far the largest dependency.
#
# Bumping SKRIBIDI_COMMIT: re-read upstream src/CMakeLists.txt for changed
# dependency pins and source lists, and mirror them here and in CMakeLists.txt.
vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

set(SKRIBIDI_COMMIT dee63d6ba76aeddd49dea6d1b2508cf9aa391f46)
set(SHEENBIDI_COMMIT 83f77108a2873600283f6da4b326a2dca7a3a7a6)
set(LIBUNIBREAK_TAG libunibreak_6_1)
set(BUDOUXC_COMMIT a044d49afc654117fac7623fff15bec15943270c)

vcpkg_download_distfile(SKRIBIDI_ARCHIVE
    URLS "https://github.com/memononen/skribidi/archive/${SKRIBIDI_COMMIT}.zip"
    FILENAME "skribidi-${SKRIBIDI_COMMIT}.zip"
    SHA512 c19f958a51d25b443b5225952d2c774c7822fff4d2bae526755cadff5d0066c4dbeb4ad2a3270695dd53f6860a322046b103a6830cb39ad5818b5e726eb8f7be
)
vcpkg_download_distfile(SHEENBIDI_ARCHIVE
    URLS "https://github.com/Tehreer/SheenBidi/archive/${SHEENBIDI_COMMIT}.zip"
    FILENAME "sheenbidi-${SHEENBIDI_COMMIT}.zip"
    SHA512 0c16b7bd19fd4768e575c06aa123391086f79c213f60983ba999e3d3e7c14503bfe1360ab68e1fc87907d97851d447bb3c182bd2113d0cccb973fb2ceda945d3
)
vcpkg_download_distfile(LIBUNIBREAK_ARCHIVE
    URLS "https://github.com/adah1972/libunibreak/archive/refs/tags/${LIBUNIBREAK_TAG}.zip"
    FILENAME "libunibreak-${LIBUNIBREAK_TAG}.zip"
    SHA512 3d97ad41dbcb3da392da4b6fbef25998db3dded24c2269653f27dd5f270d8c9598e728d760bb368dde75d0537424312cca0222863c60e71b03c3ab05c5b07ce1
)
vcpkg_download_distfile(BUDOUXC_ARCHIVE
    URLS "https://github.com/memononen/budouxc/archive/${BUDOUXC_COMMIT}.zip"
    FILENAME "budouxc-${BUDOUXC_COMMIT}.zip"
    SHA512 87a37ca9e8c92e9b6791abf2fa626cd0a55979b380bcd900f16aabb70fcf2bed2732efd85739f656f07f2ee2639b4cac532490b85bbbf01e4c062e5da7380f0e
)

vcpkg_extract_source_archive(SOURCE_PATH
    ARCHIVE "${SKRIBIDI_ARCHIVE}"
    SOURCE_BASE "skribidi-${SKRIBIDI_COMMIT}"
)
vcpkg_extract_source_archive(SHEENBIDI_SOURCE_PATH
    ARCHIVE "${SHEENBIDI_ARCHIVE}"
    SOURCE_BASE "sheenbidi-${SHEENBIDI_COMMIT}"
)
vcpkg_extract_source_archive(LIBUNIBREAK_SOURCE_PATH
    ARCHIVE "${LIBUNIBREAK_ARCHIVE}"
    SOURCE_BASE "libunibreak-${LIBUNIBREAK_TAG}"
)
vcpkg_extract_source_archive(BUDOUXC_SOURCE_PATH
    ARCHIVE "${BUDOUXC_ARCHIVE}"
    SOURCE_BASE "budouxc-${BUDOUXC_COMMIT}"
)

file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}/vcpkg")

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}/vcpkg"
    OPTIONS
        -DSKRIBIDI_SOURCE_DIR=${SOURCE_PATH}
        -DSHEENBIDI_SOURCE_DIR=${SHEENBIDI_SOURCE_PATH}
        -DLIBUNIBREAK_SOURCE_DIR=${LIBUNIBREAK_SOURCE_PATH}
        -DBUDOUXC_SOURCE_DIR=${BUDOUXC_SOURCE_PATH}
)
vcpkg_cmake_install()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_install_copyright(FILE_LIST
    "${SOURCE_PATH}/LICENSE"
    "${SHEENBIDI_SOURCE_PATH}/LICENSE"
    "${LIBUNIBREAK_SOURCE_PATH}/LICENCE"
    "${BUDOUXC_SOURCE_PATH}/LICENSE"
)

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
