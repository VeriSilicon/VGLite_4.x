cmake_minimum_required(VERSION 3.20)
set(MCUX_SDK_PROJECT_NAME vglite)
project(${MCUX_SDK_PROJECT_NAME} VERSION 1.0 LANGUAGES C)

set(SrcTopDir ${CMAKE_SOURCE_DIR})
set(ZEPHYR_BASE "$ENV{ZEPHYR_BASE}" CACHE STRING "")
# message(STATUS "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
# set(VgliteDir   ${SrcTopDir})
set(VgliteDir   ${ZEPHYR_BASE}/../modules/gpu-vglite)

if (DEFINED ENV{VGLITE_DIR})
    set(VgliteDir   $ENV{VGLITE_DIR})
endif()


set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_STATIC_LIBRARY_PREFIX lib)

message(STATUS "VgliteDir = ${VgliteDir}")

set(CMAKE_MODULE_PATH
    ${VgliteDir}/vendor/nxp/rt700_zephyr
)

add_library(vglite STATIC)
include(middleware_vglite_CPU_MIMXRT798S_CM33)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    "$<BUILD_INTERFACE:${VgliteDir}/vendor/nxp/rt700_zephyr>"
    "$<BUILD_INTERFACE:${VgliteDir}/VGLite>"
    "$<BUILD_INTERFACE:${VgliteDir}/VGLiteKernel>"
)

zephyr_include_directories(${VgliteDir})
zephyr_include_directories(${VgliteDir}/inc)

# Add VG Lite specific linker sections
zephyr_linker_sources(DATA_SECTIONS
    ${VgliteDir}/VGLiteKernel/zephyr/vglite_sections.ld)

# Define the GPU ID for all sources
add_compile_definitions(GCID_REV_CID=gc555/0x423_ECO)
target_link_libraries(vglite PRIVATE zephyr_interface)
