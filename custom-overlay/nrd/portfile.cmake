# NVIDIA NRD (Real-time Denoisers) — built from source. Unlike streamline,
# NRD has no prebuilt-binary release (github.com/NVIDIA-RTX/NRD/releases
# ships source archives only) — its CMakeLists.txt normally pulls ShaderMake
# and MathLib via live FetchContent, which vcpkg's sandboxed build forbids.
# This portfile pre-downloads those two dependencies itself and redirects
# FetchContent at the already-extracted copies (FETCHCONTENT_SOURCE_DIR_*),
# same idea as any other vcpkg port with an upstream FetchContent-based build.
#
# D3D12/DXIL only: NRD_EMBEDS_SPIRV_SHADERS and NRD_EMBEDS_DXBC_SHADERS are
# both off (no Vulkan, no legacy D3D11 FXC dependency), NRD_NRI is off
# (Spectrum drives raw D3D12 PSOs/dispatches itself, no NRI abstraction
# layer) — see HAL:NRD (mirrors HAL:Streamline's own manual-hooking style).
#
# Release-only: NRD's CMakeLists.txt sends every config's archive output to
# the SAME path (${CMAKE_SOURCE_DIR}/_Bin/NRD.lib, no per-config subdir), so
# a Debug then Release build (vcpkg's default) silently overwrites one with
# the other rather than producing two distinct libs. Building Release only
# is the standard vcpkg fix for a dependency whose build system doesn't
# separate per-config outputs — Spectrum's Debug-D3D12 links the same
# release nrd.lib as Profile/Retail, same as it would for any release-only
# vendored dependency.
set(VCPKG_BUILD_TYPE release)

set(NRD_TAG v4.17.3)
set(SHADERMAKE_COMMIT 18f5a344e7ca8fa65daaf079d07bc8ce38453e05)
set(MATHLIB_TAG v11)

vcpkg_download_distfile(NRD_ARCHIVE
    URLS "https://github.com/NVIDIA-RTX/NRD/archive/refs/tags/${NRD_TAG}.zip"
    FILENAME "nrd-${NRD_TAG}.zip"
    SHA512 167dd18af4f33e839ed9827a51d32ac714c0935fa85333082b22c403df94b953f935dd425e582a3a14a1e4001dfa84bf0015d5a4f95ded407d77ba8aeb517ad1
)
vcpkg_download_distfile(SHADERMAKE_ARCHIVE
    URLS "https://github.com/NVIDIA-RTX/ShaderMake/archive/${SHADERMAKE_COMMIT}.zip"
    FILENAME "shadermake-${SHADERMAKE_COMMIT}.zip"
    SHA512 40dd83ac68bd9062f9fb592c6566b328a65eaa1792c130d21c2e5f298fb37b0017ebe0cbe351be36e4d275883ba7bf8074539106272725c78b930e29181a3def
)
vcpkg_download_distfile(MATHLIB_ARCHIVE
    URLS "https://github.com/NVIDIA-RTX/MathLib/archive/refs/tags/${MATHLIB_TAG}.zip"
    FILENAME "mathlib-${MATHLIB_TAG}.zip"
    SHA512 513d3a62e09fdb6b1cafad8e12693fd23401b8e34bda42856b764c70f94d59b8086474ca254dd45dd258586795ed0dd3fcd1c82cea42a7dae38aabad01582eaa
)

vcpkg_extract_source_archive(NRD_SOURCE_PATH
    ARCHIVE "${NRD_ARCHIVE}"
    SOURCE_BASE "nrd-${NRD_TAG}"
)
vcpkg_extract_source_archive(SHADERMAKE_SOURCE_PATH
    ARCHIVE "${SHADERMAKE_ARCHIVE}"
    SOURCE_BASE "shadermake-${SHADERMAKE_COMMIT}"
)
vcpkg_extract_source_archive(MATHLIB_SOURCE_PATH
    ARCHIVE "${MATHLIB_ARCHIVE}"
    SOURCE_BASE "mathlib-${MATHLIB_TAG}"
)

# directx-dxc is a "host" dependency (see vcpkg.json) — its dxc.exe lands in
# the HOST triplet's installed tree, not the target one.
set(DXC_TOOL_PATH "${CURRENT_HOST_INSTALLED_DIR}/tools/directx-dxc/dxc.exe")
if(NOT EXISTS "${DXC_TOOL_PATH}")
    message(FATAL_ERROR "nrd: expected dxc.exe at '${DXC_TOOL_PATH}' from the "
                        "directx-dxc host port — its tool install layout changed, "
                        "update this path.")
endif()

vcpkg_cmake_configure(
    SOURCE_PATH "${NRD_SOURCE_PATH}"
    OPTIONS
        -DFETCHCONTENT_SOURCE_DIR_SHADERMAKE=${SHADERMAKE_SOURCE_PATH}
        -DFETCHCONTENT_SOURCE_DIR_MATHLIB=${MATHLIB_SOURCE_PATH}
        -DFETCHCONTENT_FULLY_DISCONNECTED=ON
        -DNRD_STATIC_LIBRARY=ON
        -DNRD_EMBEDS_DXIL_SHADERS=ON
        -DNRD_EMBEDS_DXBC_SHADERS=OFF
        -DNRD_EMBEDS_SPIRV_SHADERS=OFF
        -DNRD_NRI=OFF
        # ShaderMake (vendored as a nested FetchContent by NRD's own CMake)
        # normally FetchContents its OWN separate DXC copy from GitHub when
        # SHADERMAKE_FIND_COMPILERS is on (ShaderMake/CMakeLists.txt:30-91) —
        # that's a THIRD live-network fetch this sandboxed build can't do.
        # Turning off the master switch skips that whole block and lets us
        # seed SHADERMAKE_DXC_PATH directly with the dxc.exe already built
        # by the directx-dxc host port instead.
        -DSHADERMAKE_FIND_COMPILERS=OFF
        -DSHADERMAKE_DXC_PATH=${DXC_TOOL_PATH}
)

# NRD's CMakeLists.txt defines no install() rules at all (it's normally
# consumed as a FetchContent subdirectory via add_library(NRD STATIC ...),
# never as a standalone installed package) — vcpkg_cmake_install() has
# nothing to do here, so this hand-copies the built .lib and public headers
# instead, same shape as streamline's own manual file(INSTALL...) portfile.
# NRD's own CMakeLists.txt sends archive output to a single fixed
# "${CMAKE_SOURCE_DIR}/_Bin/NRD.lib" (the SOURCE tree, not vcpkg's build
# tree, and no per-config subdirectory) when built standalone, which this
# configure/build IS (NRD_SOURCE_PATH as SOURCE_PATH directly, not as a
# FetchContent-included subdirectory).
vcpkg_cmake_build(TARGET NRD)

set(NRD_LIB_PATH "${NRD_SOURCE_PATH}/_Bin/NRD.lib")
if(NOT EXISTS "${NRD_LIB_PATH}")
    message(FATAL_ERROR "nrd: expected '${NRD_LIB_PATH}' after building the NRD "
                        "target — its _Bin output layout changed, update this path.")
endif()
file(INSTALL "${NRD_LIB_PATH}" DESTINATION "${CURRENT_PACKAGES_DIR}/lib")
file(INSTALL "${NRD_LIB_PATH}" DESTINATION "${CURRENT_PACKAGES_DIR}/debug/lib")

# NRD.lib calls ShaderMake::FindPermutationInBlob() (the embedded-DXIL
# lookup helper) but doesn't bundle it — CMake's target_link_libraries
# graph would pull ShaderMakeBlob.lib in automatically for a consumer using
# find_package(nrd), but we hand-copy the .lib instead of using CMake to
# consume it, so that transitive link dependency has to be replicated here
# and in Sharpmake explicitly. Lives in the ShaderMake subbuild's own
# (unredirected) CMAKE_ARCHIVE_OUTPUT_DIRECTORY, not NRD's _Bin.
set(SHADERMAKEBLOB_LIB_PATH "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel/_deps/shadermake-build/ShaderMakeBlob.lib")
if(NOT EXISTS "${SHADERMAKEBLOB_LIB_PATH}")
    message(FATAL_ERROR "nrd: expected '${SHADERMAKEBLOB_LIB_PATH}' after building "
                        "the NRD target — ShaderMake's own build layout changed, "
                        "update this path.")
endif()
file(INSTALL "${SHADERMAKEBLOB_LIB_PATH}" DESTINATION "${CURRENT_PACKAGES_DIR}/lib")
file(INSTALL "${SHADERMAKEBLOB_LIB_PATH}" DESTINATION "${CURRENT_PACKAGES_DIR}/debug/lib")

file(GLOB NRD_HEADERS "${NRD_SOURCE_PATH}/Include/*.h")
file(INSTALL ${NRD_HEADERS} DESTINATION "${CURRENT_PACKAGES_DIR}/include/${PORT}")

vcpkg_install_copyright(FILE_LIST "${NRD_SOURCE_PATH}/LICENSE.txt")

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
