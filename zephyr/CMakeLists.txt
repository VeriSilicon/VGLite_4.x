# SPDX-License-Identifier: Apache-2.0

if(CONFIG_VGLITE)

	set(VGLITE_DIR ${ZEPHYR_CURRENT_MODULE_DIR})

	# Add VGLite include directories globally so other modules can find them
	zephyr_include_directories(${VGLITE_DIR}/inc)
	zephyr_include_directories(${VGLITE_DIR}/VGLite)
	zephyr_include_directories(${VGLITE_DIR}/VGLiteKernel)
	zephyr_include_directories(${ZEPHYR_BASE}/include/zephyr)

	# Include the vendor build system which will select the right platform
	add_subdirectory(${VGLITE_DIR}/vendor/nxp ${CMAKE_CURRENT_BINARY_DIR}/vglite)

	# Link the vglite library to the app
	zephyr_library_link_libraries(vglite)

endif()

