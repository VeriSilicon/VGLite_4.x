set(MCUX_SDK_PROJECT_NAME vglite)
project(${MCUX_SDK_PROJECT_NAME} VERSION 1.0 LANGUAGES C)

set(SrcTopDir ${CMAKE_SOURCE_DIR})

set(VgliteDir   ${SrcTopDir})
if (DEFINED ENV{VGLITE_DIR})
    set(VgliteDir   $ENV{VGLITE_DIR})
endif()


set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_STATIC_LIBRARY_PREFIX lib)

add_library(${MCUX_SDK_PROJECT_NAME} STATIC)

set(CMAKE_MODULE_PATH
    ${VgliteDir}/vendor/nxp/gen6
)

include(toolchain_flags)

include(middleware_vglite_gen6)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    "$<BUILD_INTERFACE:${VgliteDir}/vendor/nxp/gen6>"
    "$<BUILD_INTERFACE:${VgliteDir}/VGLite>"
    "$<BUILD_INTERFACE:${VgliteDir}/VGLiteKernel>"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} INTERFACE
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

install(
    FILES
    ${SrcTopDir}/inc/vg_lite.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY
    ${SrcTopDir}/VGLite/Series/
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
string(APPEND CMAKE_C_FLAGS                     " -DVG_COMMAND_CALL=1")
string(APPEND CMAKE_C_FLAGS                     " -DVG_TARGET_FAST_CLEAR=0")

# ------------------------------------------------------------------------------
# DEBUG specific compiler options
# ------------------------------------------------------------------------------
string(APPEND CMAKE_C_FLAGS_DEBUG               " -g")
string(APPEND CMAKE_C_FLAGS_DEBUG               " -O0")

string(APPEND CMAKE_C_FLAGS_DEBUG               " -DDEBUG")
string(APPEND CMAKE_C_FLAGS_DEBUG               " -DVG_RENDER_TEXT=0")

# ------------------------------------------------------------------------------
# RELEASE specific compiler options
# ------------------------------------------------------------------------------
string(APPEND CMAKE_C_FLAGS_RELEASE             " -DNDEBUG")
string(APPEND CMAKE_C_FLAGS_RELEASE             " -DVG_RENDER_TEXT=0")

# ------------------------------------------------------------------------------
# DEBUG_TEXT specific compiler options
# ------------------------------------------------------------------------------
string(APPEND CMAKE_C_FLAGS_DEBUG_TEXT          " -g")
string(APPEND CMAKE_C_FLAGS_DEBUG_TEXT          " -O0")

string(APPEND CMAKE_C_FLAGS_DEBUG_TEXT          " -DDEBUG")
string(APPEND CMAKE_C_FLAGS_DEBUG_TEXT          " -DVG_RENDER_TEXT=1")

# ------------------------------------------------------------------------------
# RELEASE_TEXT specific compiler options
# ------------------------------------------------------------------------------
string(APPEND CMAKE_C_FLAGS_RELEASE_TEXT        " -Os")

string(APPEND CMAKE_C_FLAGS_RELEASE_TEXT        " -DNDEBUG")
string(APPEND CMAKE_C_FLAGS_RELEASE_TEXT        " -DVG_RENDER_TEXT=1")
