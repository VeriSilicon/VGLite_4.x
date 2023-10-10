include_guard()
message("vglite-elementary-src component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${VgliteDir}/elementary/src/elm_buffer.c
    ${VgliteDir}/elementary/src/elm_draw.c
    ${VgliteDir}/elementary/src/elm_init.c
    ${VgliteDir}/elementary/src/elm_object.c
    ${VgliteDir}/elementary/src/elm_os.c
    ${VgliteDir}/elementary/src/elm_text.c
    ${VgliteDir}/font/buf_reader.c
    ${VgliteDir}/font/rle_font_read.c
    ${VgliteDir}/font/vft_debug.c
    ${VgliteDir}/font/vft_draw.c
    ${VgliteDir}/font/vg_lite_text.c
    ${VgliteDir}/font/mcufont/decoder/mf_bwfont.c
    ${VgliteDir}/font/mcufont/decoder/mf_encoding.c
    ${VgliteDir}/font/mcufont/decoder/mf_font.c
    ${VgliteDir}/font/mcufont/decoder/mf_justify.c
    ${VgliteDir}/font/mcufont/decoder/mf_kerning.c
    ${VgliteDir}/font/mcufont/decoder/mf_rlefont.c
    ${VgliteDir}/font/mcufont/decoder/mf_scaledfont.c
    ${VgliteDir}/font/mcufont/decoder/mf_wordwrap.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${VgliteDir}/elementary/inc
    ${VgliteDir}/elementary/src
)
