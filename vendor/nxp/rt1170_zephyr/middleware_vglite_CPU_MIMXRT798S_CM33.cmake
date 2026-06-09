include_guard()
message("vglite-src component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${VgliteDir}/VGLite/vg_lite.c
    ${VgliteDir}/VGLite/vg_lite_image.c
    ${VgliteDir}/VGLite/vg_lite_matrix.c
    ${VgliteDir}/VGLite/vg_lite_path.c
    ${VgliteDir}/VGLite/zephyr/vg_lite_os.c
    ${VgliteDir}/VGLite/vg_lite_stroke.c
	${VgliteDir}/VGLite/vg_lite_chip.c
    ${VgliteDir}/VGLiteKernel/vg_lite_kernel.c
    ${VgliteDir}/VGLiteKernel/zephyr/vg_lite_hal.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${VgliteDir}/inc
    ${VgliteDir}/VGLite/zephyr
    ${VgliteDir}/VGLiteKernel
    ${VgliteDir}/VGLiteKernel/zephyr    
)


