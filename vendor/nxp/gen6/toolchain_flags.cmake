

# ------------------------------------------------------------------------------
# Common Compiler Options
# ------------------------------------------------------------------------------
string(APPEND CMAKE_ASM_FLAGS                           " -Wall")
# string(APPEND CMAKE_ASM_FLAGS                           " -fno-common")
# string(APPEND CMAKE_ASM_FLAGS                           " -ffunction-sections")
# string(APPEND CMAKE_ASM_FLAGS                           " -fdata-sections")
# string(APPEND CMAKE_ASM_FLAGS                           " -ffreestanding")
# string(APPEND CMAKE_ASM_FLAGS                           " -fno-builtin")
# string(APPEND CMAKE_ASM_FLAGS                           " -mthumb")
# string(APPEND CMAKE_ASM_FLAGS                           " -mapcs")
# string(APPEND CMAKE_ASM_FLAGS                           " -std=gnu99")
# string(APPEND CMAKE_ASM_FLAGS                           " -mcpu=cortex-m7")
# string(APPEND CMAKE_ASM_FLAGS                           " -mfloat-abi=hard")
# string(APPEND CMAKE_ASM_FLAGS                           " -mfpu=fpv5-d16")

# string(APPEND CMAKE_ASM_FLAGS                           " -D__STARTUP_CLEAR_BSS")
# string(APPEND CMAKE_ASM_FLAGS                           " -D__STARTUP_INITIALIZE_NONCACHEDATA")

string(APPEND CMAKE_C_FLAGS                             " -DGCID_REV_CID=gc555/0x423_ECO")
string(APPEND CMAKE_C_FLAGS                             " -Wall")
# string(APPEND CMAKE_C_FLAGS                             " -fno-common")
# string(APPEND CMAKE_C_FLAGS                             " -ffunction-sections")
# string(APPEND CMAKE_C_FLAGS                             " -fdata-sections")
# string(APPEND CMAKE_C_FLAGS                             " -ffreestanding")
# string(APPEND CMAKE_C_FLAGS                             " -fno-builtin")
# string(APPEND CMAKE_C_FLAGS                             " -mthumb")
# string(APPEND CMAKE_C_FLAGS                             " -mapcs")
string(APPEND CMAKE_C_FLAGS                             " -std=gnu99")
# string(APPEND CMAKE_C_FLAGS                             " -mcpu=cortex-m7")
# string(APPEND CMAKE_C_FLAGS                             " -mfloat-abi=hard")
# string(APPEND CMAKE_C_FLAGS                             " -mfpu=fpv5-d16")
# string(APPEND CMAKE_C_FLAGS                             " -MMD")
# string(APPEND CMAKE_C_FLAGS                             " -MP")

# string(APPEND CMAKE_C_FLAGS                             " -DCPU_MIMXRT1176DVMAA_cm7")
# string(APPEND CMAKE_C_FLAGS                             " -DUSE_SDRAM")
# string(APPEND CMAKE_C_FLAGS                             " -DDATA_SECTION_IS_CACHEABLE=1")
# string(APPEND CMAKE_C_FLAGS                             " -DSDK_DEBUGCONSOLE_UART")
# string(APPEND CMAKE_C_FLAGS                             " -DSERIAL_PORT_TYPE_UART=1")
# string(APPEND CMAKE_C_FLAGS                             " -DRTOS")
# string(APPEND CMAKE_C_FLAGS                             " -DSDK_OS_FREE_RTOS")
# string(APPEND CMAKE_C_FLAGS                             " -DFSL_SDK_DRIVER_QUICK_ACCESS_ENABLE=1")
# string(APPEND CMAKE_C_FLAGS                             " -DMCUXPRESSO_SDK")
# string(APPEND CMAKE_C_FLAGS                             " -DSD_ENABLED")
# string(APPEND CMAKE_C_FLAGS                             " -DAPP_ENABLE_SDCARD")


# ------------------------------------------------------------------------------
# SDRAM_DEBUG Compiler Options
# ------------------------------------------------------------------------------
#string(APPEND CMAKE_ASM_FLAGS_SDRAM_DEBUG               " -g")

#string(APPEND CMAKE_ASM_FLAGS_SDRAM_DEBUG               " -DDEBUG")

#string(APPEND CMAKE_C_FLAGS_SDRAM_DEBUG                 " -g")
#string(APPEND CMAKE_C_FLAGS_SDRAM_DEBUG                 " -O0")

#string(APPEND CMAKE_C_FLAGS_SDRAM_DEBUG                 " -DDEBUG")


# ------------------------------------------------------------------------------
# SDRAM_RELEASE Compiler Options
# ------------------------------------------------------------------------------
#string(APPEND CMAKE_ASM_FLAGS_SDRAM_RELEASE             " -DNDEBUG")

#string(APPEND CMAKE_C_FLAGS_SDRAM_RELEASE               " -Os")

#string(APPEND CMAKE_C_FLAGS_SDRAM_RELEASE               " -DNDEBUG")


# ------------------------------------------------------------------------------
# SDRAM_DEBUG_CUSTOM_MEM Compiler Options
# ------------------------------------------------------------------------------
# string(APPEND CMAKE_C_FLAGS_SDRAM_DEBUG_CUSTOM_MEM      " -g")
# string(APPEND CMAKE_C_FLAGS_SDRAM_DEBUG_CUSTOM_MEM      " -O0")

# string(APPEND CMAKE_C_FLAGS_SDRAM_DEBUG_CUSTOM_MEM      " -DDEBUG")
# string(APPEND CMAKE_C_FLAGS_SDRAM_DEBUG_CUSTOM_MEM      " -DCUSTOM_VGLITE_MEMORY_CONFIG=1")


# ------------------------------------------------------------------------------
# SDRAM_RELEASE_CUSTOM_MEM Compiler Options
# ------------------------------------------------------------------------------
# string(APPEND CMAKE_C_FLAGS_SDRAM_RELEASE_CUSTOM_MEM    " -Os")

# string(APPEND CMAKE_C_FLAGS_SDRAM_RELEASE_CUSTOM_MEM    " -DNDEBUG")
# string(APPEND CMAKE_C_FLAGS_SDRAM_RELEASE_CUSTOM_MEM    " -DCUSTOM_VGLITE_MEMORY_CONFIG=1")


# ------------------------------------------------------------------------------
# Common Linker Options
# ------------------------------------------------------------------------------
# string(APPEND CMAKE_EXE_LINKER_FLAGS                    " --specs=nano.specs")
# string(APPEND CMAKE_EXE_LINKER_FLAGS                    " --specs=nosys.specs")
string(APPEND CMAKE_EXE_LINKER_FLAGS                    " -Wall")
# string(APPEND CMAKE_EXE_LINKER_FLAGS                    " -fno-common")
# string(APPEND CMAKE_EXE_LINKER_FLAGS                    " -ffunction-sections")
# string(APPEND CMAKE_EXE_LINKER_FLAGS                    " -fdata-sections")
# string(APPEND CMAKE_EXE_LINKER_FLAGS                    " -ffreestanding")
# string(APPEND CMAKE_EXE_LINKER_FLAGS                    " -fno-builtin")
# string(APPEND CMAKE_EXE_LINKER_FLAGS                    " -mthumb")
# string(APPEND CMAKE_EXE_LINKER_FLAGS                    " -mapcs")
# string(APPEND CMAKE_EXE_LINKER_FLAGS                    " -Xlinker -static")
# string(APPEND CMAKE_EXE_LINKER_FLAGS                    " -Xlinker -zmuldefs")
# string(APPEND CMAKE_EXE_LINKER_FLAGS                    " -Xlinker --gc-sections")
# string(APPEND CMAKE_EXE_LINKER_FLAGS                    " -Xlinker -Map=output.map")
# string(APPEND CMAKE_EXE_LINKER_FLAGS                    " -Xlinker --print-memory-usage")
# string(APPEND CMAKE_EXE_LINKER_FLAGS                    " -Xlinker --sort-section=alignment")
# string(APPEND CMAKE_EXE_LINKER_FLAGS                    " -mcpu=cortex-m7")
# string(APPEND CMAKE_EXE_LINKER_FLAGS                    " -mfloat-abi=hard")
# string(APPEND CMAKE_EXE_LINKER_FLAGS                    " -mfpu=fpv5-d16")


# ------------------------------------------------------------------------------
# SDRAM_DEBUG Linker Options
# ------------------------------------------------------------------------------
# string(APPEND CMAKE_EXE_LINKER_FLAGS_SDRAM_DEBUG        " -g")
