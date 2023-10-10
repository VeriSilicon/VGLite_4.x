set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)

set(VGLITE_PLATFORM   "rt1170" CACHE STRING "Compile VGLite for this platform.")
set_property(CACHE VGLITE_PLATFORM PROPERTY STRINGS "rt500;rt1170")

set(MCU_SDK_VER   "2.13.x" CACHE STRING "Compile for this version of MCU-SDK.")
set_property(CACHE MCU_SDK_VER PROPERTY STRINGS "2.13.x")

set(SrcTopDir ${CMAKE_SOURCE_DIR})

add_subdirectory(${VGLITE_PLATFORM})
