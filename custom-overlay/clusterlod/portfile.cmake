set(VCPKG_BUILD_TYPE release)

set(CLUSTERLOD_COMMIT 034c84ccf641701ddbaa628a29a5e76c222de51b)

vcpkg_download_distfile(CLUSTERLOD_HEADER
    URLS "https://raw.githubusercontent.com/zeux/meshoptimizer/${CLUSTERLOD_COMMIT}/demo/clusterlod.h"
    FILENAME "clusterlod-${CLUSTERLOD_COMMIT}.h"
    SHA512 883fd7ecdcd60678ef379c48ba6e3b5c98e3bfc6b4f7f88cad20ab4897ea0cd166be00ef448a5860463ca9ccb4508b42dc501858ba4d6140ec226a733eb57e81
)
vcpkg_download_distfile(CLUSTERLOD_LICENSE
    URLS "https://raw.githubusercontent.com/zeux/meshoptimizer/${CLUSTERLOD_COMMIT}/LICENSE.md"
    FILENAME "clusterlod-license-${CLUSTERLOD_COMMIT}.md"
    SHA512 900d0696ce537ceeadea65f3f1aaee0daa5ec2e69cfb1c2b26c27bccbc4ab108f3cd5ffeea4c5fdd5065891316bd41863551cf8902a3c48f2a6d774c24b8e5e1
)

file(INSTALL "${CLUSTERLOD_HEADER}" DESTINATION "${CURRENT_PACKAGES_DIR}/include" RENAME "clusterlod.h")
file(INSTALL "${CLUSTERLOD_LICENSE}" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME "copyright")
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
