set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# OS choice
set(VGLITE_OS "freertos" CACHE STRING "Target OS for VGLite build")
set_property(CACHE VGLITE_OS PROPERTY STRINGS "zephyr;freertos")

set(VGLITE_PLATFORM   "rt1170" CACHE STRING "Compile VGLite for this platform.")
set_property(CACHE VGLITE_PLATFORM PROPERTY STRINGS "rt500;rt1170;gen6;rt700")

set(MCU_SDK_VER   "2.13.x" CACHE STRING "Compile for this version of MCU-SDK.")
set_property(CACHE MCU_SDK_VER PROPERTY STRINGS "2.13.x;2.15.x;2.16.x;linux")

set(SrcTopDir ${CMAKE_SOURCE_DIR})

# Build path selection
if(VGLITE_OS STREQUAL "zephyr")
    add_subdirectory(${VGLITE_PLATFORM}_zephyr)
elseif(VGLITE_OS STREQUAL "freertos")
    add_subdirectory(${VGLITE_PLATFORM})
else()
    message(FATAL_ERROR "Unsupported OS: ${VGLITE_OS}")
endif()
