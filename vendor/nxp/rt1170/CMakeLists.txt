set(MCUX_SDK_PROJECT_NAME vglite)
project(${MCUX_SDK_PROJECT_NAME} VERSION 1.0 LANGUAGES C)

set(SrcTopDir ${CMAKE_SOURCE_DIR})
set(SdkTopDir "")
if (DEFINED ENV{MCU_SDK_DIR})
    set(SdkTopDir   $ENV{MCU_SDK_DIR})
endif()

set(VgliteDir   ${SrcTopDir})
if (DEFINED ENV{VGLITE_DIR})
    set(VgliteDir   $ENV{VGLITE_DIR})
endif()


set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_STATIC_LIBRARY_PREFIX lib)

add_library(${MCUX_SDK_PROJECT_NAME} STATIC)

set(CMAKE_MODULE_PATH
    ${VgliteDir}/vendor/nxp/rt1170
    ${SdkTopDir}/boards/evkmimxrt1170/vglite_acceptance/armgcc
    ${SdkTopDir}/rtos/freertos/freertos-kernel/
)

include(toolchain_flags)

include(middleware_vglite_MIMXRT1176_cm7)
#include(middleware_vglite_elementary_MIMXRT1176_cm7)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    "$<BUILD_INTERFACE:${VgliteDir}/vendor/nxp/rt1170>"
    "$<BUILD_INTERFACE:${VgliteDir}/VGLite>"
    ${SdkTopDir}/rtos/freertos/freertos-kernel/include
    ${SdkTopDir}/rtos/freertos/freertos-kernel/portable/GCC/ARM_CM4F
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} INTERFACE
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

install(
    FILES
    ${SrcTopDir}/inc/vg_lite.h
    ${SrcTopDir}/VGLiteKernel/rtos/vg_lite_platform.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    TARGETS ${MCUX_SDK_PROJECT_NAME}
    EXPORT VGLiteTargets
    LIBRARY DESTINATION  ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION  ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION  ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT VGLiteTargets
    FILE VGLiteTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vglite
)

# ------------------------------------------------------------------------------
# VGLite specific symbol definitions
# ------------------------------------------------------------------------------
# string(APPEND CMAKE_C_FLAGS                   " -DVG_RESOLVE_ENGINE=0")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_PE_COLOR_KEY=0")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_IM_INDEX_FORMAT=0")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_DOUBLE_IMAGE=0")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_AYUV_INPUT_OUTPUT=0")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_RECTANGLE_STRIP_MODE=0")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_MMU=0")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_IM_FILTER=0")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_IM_YUV_PACKET=1")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_IM_YUV_PLANAR=0")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_PE_YUV_PACKET=1")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_TARGET_TILED=1")
string(APPEND CMAKE_C_FLAGS                     " -DVG_COMMAND_CALL=1")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_COMMAND_CALL=1")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_SHARE_BUFFER_IM_16K=0")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_OFFLINE_MODE=0")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_RESOLUTION_2880=0")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_PE_PREMULTIPLY=0")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_POST_CONVERTER=0")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_PRE_CONVERTER=0")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_RENDER_BY_MESH=0")
string(APPEND CMAKE_C_FLAGS                     " -DVG_TARGET_FAST_CLEAR=0")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_BUFFER_NUMBER_OF_TARGET=0")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_VIDEO_CLEAR_CONTROL=0")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_VIDEO_CONTROL=0")
# string(APPEND CMAKE_C_FLAGS                   " -DVGLITE_TST_FIRMWARE=0")
# string(APPEND CMAKE_C_FLAGS                   " -DVG_LITE_SYS_GPU_CTRL=0")


# ------------------------------------------------------------------------------
# SDRAM_DEBUG specific compiler options
# ------------------------------------------------------------------------------
string(APPEND CMAKE_C_FLAGS_SDRAM_DEBUG         " -DVG_RENDER_TEXT=0")

# ------------------------------------------------------------------------------
# SDRAM_RELEASE specific compiler options
# ------------------------------------------------------------------------------
string(APPEND CMAKE_C_FLAGS_SDRAM_RELEASE       " -DVG_RENDER_TEXT=0")

# ------------------------------------------------------------------------------
# SDRAM_DEBUG_TEXT specific compiler options
# ------------------------------------------------------------------------------
string(APPEND CMAKE_C_FLAGS_SDRAM_DEBUG_TEXT    " -g")
string(APPEND CMAKE_C_FLAGS_SDRAM_DEBUG_TEXT    " -O0")

string(APPEND CMAKE_C_FLAGS_SDRAM_DEBUG_TEXT    " -DDEBUG")
string(APPEND CMAKE_C_FLAGS_SDRAM_DEBUG_TEXT    " -DVG_RENDER_TEXT=1")


# ------------------------------------------------------------------------------
# SDRAM_RELEASE_TEXT specific compiler options
# ------------------------------------------------------------------------------
string(APPEND CMAKE_C_FLAGS_SDRAM_RELEASE_TEXT  " -Os")

string(APPEND CMAKE_C_FLAGS_SDRAM_RELEASE_TEXT  " -DNDEBUG")
string(APPEND CMAKE_C_FLAGS_SDRAM_RELEASE_TEXT  " -DVG_RENDER_TEXT=1")
