
# ------------------------------------------------------------------------------
# Common Compiler Options
# ------------------------------------------------------------------------------
string(APPEND CMAKE_ASM_FLAGS                   " -mcpu=cortex-m33")
string(APPEND CMAKE_ASM_FLAGS                   " -Wall")
string(APPEND CMAKE_ASM_FLAGS                   " -mfloat-abi=hard")
string(APPEND CMAKE_ASM_FLAGS                   " -mfpu=fpv5-sp-d16")
string(APPEND CMAKE_ASM_FLAGS                   " -mthumb")
string(APPEND CMAKE_ASM_FLAGS                   " -fno-common")
string(APPEND CMAKE_ASM_FLAGS                   " -ffunction-sections")
string(APPEND CMAKE_ASM_FLAGS                   " -fdata-sections")
string(APPEND CMAKE_ASM_FLAGS                   " -ffreestanding")
string(APPEND CMAKE_ASM_FLAGS                   " -fno-builtin")
string(APPEND CMAKE_ASM_FLAGS                   " -mapcs")
string(APPEND CMAKE_ASM_FLAGS                   " -std=c11")

string(APPEND CMAKE_ASM_FLAGS                   " -D__STARTUP_CLEAR_BSS")

string(APPEND CMAKE_C_FLAGS                     " -mcpu=cortex-m33")
string(APPEND CMAKE_C_FLAGS                     " -Wall")
string(APPEND CMAKE_C_FLAGS                     " -mfloat-abi=hard")
string(APPEND CMAKE_C_FLAGS                     " -mfpu=fpv5-sp-d16")
string(APPEND CMAKE_C_FLAGS                     " -mthumb")
string(APPEND CMAKE_C_FLAGS                     " -fno-common")
string(APPEND CMAKE_C_FLAGS                     " -ffunction-sections")
string(APPEND CMAKE_C_FLAGS                     " -fdata-sections")
string(APPEND CMAKE_C_FLAGS                     " -ffreestanding")
string(APPEND CMAKE_C_FLAGS                     " -fno-builtin")
string(APPEND CMAKE_C_FLAGS                     " -mapcs")
string(APPEND CMAKE_C_FLAGS                     " -std=c11")
string(APPEND CMAKE_C_FLAGS                     " -fstack-usage")
string(APPEND CMAKE_C_FLAGS                     " -MMD")
string(APPEND CMAKE_C_FLAGS                     " -MP")

string(APPEND CMAKE_C_FLAGS                     " -DDEBUG")
string(APPEND CMAKE_C_FLAGS                     " -DCPU_MIMXRT595SFFOC_cm33")
string(APPEND CMAKE_C_FLAGS                     " -DBOOT_HEADER_ENABLE=1")
string(APPEND CMAKE_C_FLAGS                     " -DBOARD_ENABLE_PSRAM_CACHE=0")
string(APPEND CMAKE_C_FLAGS                     " -DSSD1963_DATA_WITDH=8")
string(APPEND CMAKE_C_FLAGS                     " -DFLEXIO_MCULCD_DATA_BUS_WIDTH=8")
string(APPEND CMAKE_C_FLAGS                     " -DSDK_DEBUGCONSOLE_UART")
string(APPEND CMAKE_C_FLAGS                     " -DRTOS")
string(APPEND CMAKE_C_FLAGS                     " -DSERIAL_PORT_TYPE_UART=1")
string(APPEND CMAKE_C_FLAGS                     " -DSDK_OS_FREE_RTOS")
string(APPEND CMAKE_C_FLAGS                     " -DMCUXPRESSO_SDK")
string(APPEND CMAKE_C_FLAGS                     " -DGCID_REV_CID=gc255/0x40A")


# ------------------------------------------------------------------------------
# DEBUG Compiler Options
# ------------------------------------------------------------------------------
string(APPEND CMAKE_ASM_FLAGS_DEBUG             " -g3")
string(APPEND CMAKE_ASM_FLAGS_DEBUG             " -O0")
string(APPEND CMAKE_ASM_FLAGS_DEBUG             " -DDEBUG")

string(APPEND CMAKE_C_FLAGS_DEBUG               " -g3")
string(APPEND CMAKE_C_FLAGS_DEBUG               " -O0")
string(APPEND CMAKE_C_FLAGS_DEBUG               " -DDEBUG")


# ------------------------------------------------------------------------------
# RELEASE Compiler Options
# ------------------------------------------------------------------------------
string(APPEND CMAKE_ASM_FLAGS_RELEASE           " -Os")
string(APPEND CMAKE_ASM_FLAGS_RELEASE           " -DNDEBUG")

string(APPEND CMAKE_C_FLAGS_RELEASE             " -Os")
string(APPEND CMAKE_C_FLAGS_RELEASE             " -DNDEBUG")


# ------------------------------------------------------------------------------
# DEBUG_CUSTOM_MEM Compiler Options
# ------------------------------------------------------------------------------
string(APPEND CMAKE_C_FLAGS_DEBUG_CUSTOM_MEM    " -g3")
string(APPEND CMAKE_C_FLAGS_DEBUG_CUSTOM_MEM    " -O0")

string(APPEND CMAKE_C_FLAGS_DEBUG_CUSTOM_MEM    " -DDEBUG")
string(APPEND CMAKE_C_FLAGS_DEBUG_CUSTOM_MEM    " -DCUSTOM_VGLITE_MEMORY_CONFIG=1")


# ------------------------------------------------------------------------------
# RELEASE_CUSTOM_MEM Compiler Options
# ------------------------------------------------------------------------------
string(APPEND CMAKE_C_FLAGS_RELEASE_CUSTOM_MEM  " -Os")

string(APPEND CMAKE_C_FLAGS_RELEASE_CUSTOM_MEM  " -DNDEBUG")
string(APPEND CMAKE_C_FLAGS_RELEASE_CUSTOM_MEM  " -DCUSTOM_VGLITE_MEMORY_CONFIG=1")


# ------------------------------------------------------------------------------
# Common Linker Options
# ------------------------------------------------------------------------------
string(APPEND CMAKE_EXE_LINKER_FLAGS            " -mcpu=cortex-m33")
string(APPEND CMAKE_EXE_LINKER_FLAGS            " -Wall")
string(APPEND CMAKE_EXE_LINKER_FLAGS            " -mfloat-abi=hard")
string(APPEND CMAKE_EXE_LINKER_FLAGS            " -mfpu=fpv5-sp-d16")
string(APPEND CMAKE_EXE_LINKER_FLAGS            " -fno-common")
string(APPEND CMAKE_EXE_LINKER_FLAGS            " -ffunction-sections")
string(APPEND CMAKE_EXE_LINKER_FLAGS            " -fdata-sections")
string(APPEND CMAKE_EXE_LINKER_FLAGS            " -ffreestanding")
string(APPEND CMAKE_EXE_LINKER_FLAGS            " -fno-builtin")
string(APPEND CMAKE_EXE_LINKER_FLAGS            " -mthumb")
string(APPEND CMAKE_EXE_LINKER_FLAGS            " -mapcs")
string(APPEND CMAKE_EXE_LINKER_FLAGS            " -Xlinker --gc-sections")
string(APPEND CMAKE_EXE_LINKER_FLAGS            " -Xlinker -static")
string(APPEND CMAKE_EXE_LINKER_FLAGS            " -Xlinker -zmuldefs")
string(APPEND CMAKE_EXE_LINKER_FLAGS            " -Xlinker -Map=output.map")
string(APPEND CMAKE_EXE_LINKER_FLAGS            " -Xlinker --cref")
string(APPEND CMAKE_EXE_LINKER_FLAGS            " -Xlinker --print-memory-usage")
string(APPEND CMAKE_EXE_LINKER_FLAGS            " -Xlinker --sort-section=alignment")
string(APPEND CMAKE_EXE_LINKER_FLAGS            " --specs=nano.specs")
string(APPEND CMAKE_EXE_LINKER_FLAGS            " --specs=nosys.specs")


# ------------------------------------------------------------------------------
# DEBUG Linker Options
# ------------------------------------------------------------------------------
string(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG      " -g3")
string(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG      " -O0")


# ------------------------------------------------------------------------------
# RELEASE Linker Options
# ------------------------------------------------------------------------------
string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE    " -Os")
