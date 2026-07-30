# NVIDIA Streamline SDK — binary distribution port. Only sl.interposer.lib is
# linked; every plugin/NGX model DLL is LoadLibrary'd at runtime instead.
set(VCPKG_POLICY_DLLS_IN_STATIC_LIBRARY enabled)
set(VCPKG_POLICY_MISMATCHED_NUMBER_OF_BINARIES enabled)

set(SL_VERSION 2.12.0)

vcpkg_download_distfile(ARCHIVE
    URLS "https://github.com/NVIDIA-RTX/Streamline/releases/download/v${SL_VERSION}/streamline-sdk-v${SL_VERSION}.zip"
    FILENAME "streamline-sdk-v${SL_VERSION}.zip"
    SHA512 c69e620d4dec616e1c22439d32ad54bc6d82d6d5fab085ceeeb312f61d9dba23465dae80ac668135742300bb7748683e71568d3585a63c3b6c599a20ce824b0e
)

vcpkg_extract_source_archive(
    PACKAGE_PATH
    ARCHIVE "${ARCHIVE}"
    NO_REMOVE_ONE_LEVEL
)

# --- component selection ---------------------------------------------------
# Only DLSS Super Resolution + Ray Reconstruction; installing the full
# bin/x64 set would drag in ~110 MB of unused features.
#
# Deliberately NOT installed — add to the list if the feature is ever adopted:
#   sl.dlss_g.dll / nvngx_dlssg.dll   Frame Generation (needs the full interposer)
#   sl.reflex.dll / sl.pcl.dll / NvLowLatencyVk.dll   Reflex + latency stats
#   sl.nis.dll                        NVIDIA Image Scaling
#   sl.deepdvc.dll / nvngx_deepdvc.dll  DeepDVC
#   sl.directsr.dll                   DirectSR
#   sl.nvperf.dll                     Nsight Perf instrumentation
set(SL_RUNTIME
    sl.interposer.dll     # sl* entry points, plugin loader
    sl.common.dll         # shared plugin infrastructure, required by all features
    sl.dlss.dll           # DLSS Super Resolution
    nvngx_dlss.dll        #   ...its NGX model
    sl.dlss_d.dll         # DLSS Ray Reconstruction
    nvngx_dlssd.dll       #   ...its NGX model
)

# The development DLLs additionally want these two present next to them.
set(SL_RUNTIME_DEV_EXTRA
    sl.imgui.dll              # on-screen SL diagnostics overlay
    WinPixEventRuntime.dll    # development builds emit PIX markers
)

set(SL_BIN_PROD "${PACKAGE_PATH}/bin/x64")
set(SL_BIN_DEV  "${PACKAGE_PATH}/bin/x64/development")

foreach(dir IN ITEMS "${SL_BIN_PROD}" "${SL_BIN_DEV}" "${PACKAGE_PATH}/include" "${PACKAGE_PATH}/lib/x64")
    if(NOT EXISTS "${dir}")
        message(FATAL_ERROR "streamline: expected '${dir}' in the release archive. "
                            "The v${SL_VERSION} layout differs from what this portfile assumes — "
                            "inspect the extracted tree and update the paths above.")
    endif()
endforeach()

function(sl_collect out_var dir)
    set(_found "")
    foreach(name IN LISTS ARGN)
        if(EXISTS "${dir}/${name}")
            list(APPEND _found "${dir}/${name}")
        else()
            message(FATAL_ERROR "streamline: '${name}' not found in ${dir}. "
                                "Component names changed in v${SL_VERSION}.")
        endif()
    endforeach()
    set(${out_var} "${_found}" PARENT_SCOPE)
endfunction()

sl_collect(SL_PROD_FILES "${SL_BIN_PROD}" ${SL_RUNTIME})
sl_collect(SL_DEV_FILES  "${SL_BIN_DEV}"  ${SL_RUNTIME} ${SL_RUNTIME_DEV_EXTRA})

# --- install ---------------------------------------------------------------

file(GLOB SL_HEADERS "${PACKAGE_PATH}/include/*.h")
file(INSTALL ${SL_HEADERS} DESTINATION "${CURRENT_PACKAGES_DIR}/include/${PORT}")

file(INSTALL "${PACKAGE_PATH}/lib/x64/sl.interposer.lib" DESTINATION "${CURRENT_PACKAGES_DIR}/lib")
file(INSTALL ${SL_PROD_FILES} DESTINATION "${CURRENT_PACKAGES_DIR}/bin")

# Debug gets the development set, so Debug-D3D12 builds pick up SL's verbose
# logging and overlay without any code change (Sharpmake copies from debug/bin).
if(NOT DEFINED VCPKG_BUILD_TYPE)
    file(INSTALL "${PACKAGE_PATH}/lib/x64/sl.interposer.lib" DESTINATION "${CURRENT_PACKAGES_DIR}/debug/lib")
    file(INSTALL ${SL_DEV_FILES} DESTINATION "${CURRENT_PACKAGES_DIR}/debug/bin")
endif()

# --- licence ---------------------------------------------------------------
# Mixed licensing: the Streamline framework is MIT, the bundled nvngx_dlss*
# models are under the NVIDIA DLSS licence, hence "license": null in vcpkg.json.
vcpkg_install_copyright(FILE_LIST
    "${PACKAGE_PATH}/license.txt"
    "${PACKAGE_PATH}/3rd-party-licenses.md"
    "${SL_BIN_PROD}/nvngx_dlss.license.txt"
)

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
