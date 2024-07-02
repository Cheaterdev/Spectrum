vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Cheaterdev/Jinja2Cpp
    REF "${VERSION}"
    SHA512 0fd1cede08e8617af80a8cdbf27d9ed54ee34efd975c884b3aeb94a21d9fade38b6a9c300ffbfa55a7430dfe7550f6c0c8ce98172d96feaa63b3cafb30c262de
    HEAD_REF master
    PATCHES
        build_fixes.patch
)

string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "dynamic" JINJA2CPP_BUILD_SHARED)

vcpkg_cmake_configure(
  SOURCE_PATH ${SOURCE_PATH}
  OPTIONS
    -DJINJA2CPP_BUILD_TESTS=OFF
    -DJINJA2CPP_STRICT_WARNINGS=OFF
    -DJINJA2CPP_BUILD_SHARED=${JINJA2CPP_BUILD_SHARED}
    -DCMAKE_DISABLE_FIND_PACKAGE_expected-lite=ON
    -DCMAKE_DISABLE_FIND_PACKAGE_variant-lite=ON
    -DCMAKE_DISABLE_FIND_PACKAGE_optional-lite=ON
    -DCMAKE_DISABLE_FIND_PACKAGE_string-view-lite=ON
    -DJINJA2CPP_DEPS_MODE=external
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH "lib/${PORT}")

file(MAKE_DIRECTORY "${CURRENT_PACKAGES_DIR}/lib/pkgconfig" "${CURRENT_PACKAGES_DIR}/debug/lib/pkgconfig")
file(RENAME "${CURRENT_PACKAGES_DIR}/debug/share/pkgconfig/jinja2cpp.pc" "${CURRENT_PACKAGES_DIR}/debug/lib/pkgconfig/jinja2cpp.pc")
file(RENAME "${CURRENT_PACKAGES_DIR}/share/pkgconfig/jinja2cpp.pc" "${CURRENT_PACKAGES_DIR}/lib/pkgconfig/jinja2cpp.pc")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/share/pkgconfig")

vcpkg_fixup_pkgconfig()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")