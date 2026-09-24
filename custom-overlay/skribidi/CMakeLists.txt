# Replacement build for Skribidi; see portfile.cmake for why upstream's is not used.
# Source lists mirror upstream src/CMakeLists.txt at the pinned commit.
cmake_minimum_required(VERSION 3.22)
project(skribidi LANGUAGES C)

include(GNUInstallDirs)

find_package(harfbuzz CONFIG REQUIRED)

set(SKB_SRC "${SKRIBIDI_SOURCE_DIR}/src")
set(UNIBREAK_SRC "${LIBUNIBREAK_SOURCE_DIR}/src")

add_library(skribidi STATIC
    ${SKB_SRC}/skb_attributes.c
    ${SKB_SRC}/skb_attribute_collection.c
    ${SKB_SRC}/skb_canvas.c
    ${SKB_SRC}/skb_common.c
    ${SKB_SRC}/skb_editor.c
    ${SKB_SRC}/skb_editor_rules.c
    ${SKB_SRC}/skb_font_collection.c
    ${SKB_SRC}/skb_icon_collection.c
    ${SKB_SRC}/skb_image_atlas.c
    ${SKB_SRC}/skb_layout.c
    ${SKB_SRC}/skb_layout_cache.c
    ${SKB_SRC}/skb_rasterizer.c
    ${SKB_SRC}/skb_rich_layout.c
    ${SKB_SRC}/skb_rich_text.c
    ${SKB_SRC}/skb_text.c

    # SheenBidi, unity build (upstream's default SB_CONFIG_UNITY=ON)
    ${SHEENBIDI_SOURCE_DIR}/Source/SheenBidi.c

    ${UNIBREAK_SRC}/eastasianwidthdata.c
    ${UNIBREAK_SRC}/eastasianwidthdef.c
    ${UNIBREAK_SRC}/emojidata.c
    ${UNIBREAK_SRC}/emojidef.c
    ${UNIBREAK_SRC}/graphemebreak.c
    ${UNIBREAK_SRC}/graphemebreakdata.c
    ${UNIBREAK_SRC}/indicconjunctbreakdata.c
    ${UNIBREAK_SRC}/linebreak.c
    ${UNIBREAK_SRC}/linebreakdata.c
    ${UNIBREAK_SRC}/linebreakdef.c
    ${UNIBREAK_SRC}/unibreakbase.c
    ${UNIBREAK_SRC}/unibreakdef.c
    ${UNIBREAK_SRC}/wordbreak.c
    ${UNIBREAK_SRC}/wordbreakdata.c

    ${BUDOUXC_SOURCE_DIR}/src/budoux.c
)

target_include_directories(skribidi
    PUBLIC
        $<BUILD_INTERFACE:${SKRIBIDI_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${SKB_SRC}
        ${SHEENBIDI_SOURCE_DIR}/Headers
        ${UNIBREAK_SRC}
        ${BUDOUXC_SOURCE_DIR}/include
        ${BUDOUXC_SOURCE_DIR}/src
)

target_compile_definitions(skribidi PRIVATE SB_CONFIG_UNITY)

if(MSVC)
    # SheenBidi's own CMake builds with /utf-8.
    target_compile_options(skribidi PRIVATE /utf-8)
endif()

set_target_properties(skribidi PROPERTIES
    C_STANDARD 17
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS YES
)

target_link_libraries(skribidi PRIVATE harfbuzz::harfbuzz)

install(TARGETS skribidi
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY "${SKRIBIDI_SOURCE_DIR}/include/skribidi" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
