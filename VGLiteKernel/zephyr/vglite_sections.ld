/* VG Lite specific memory sections for Zephyr */

SECTION_DATA_PROLOGUE(.vglite.noinit.mem_pool,,)
{
    . = ALIGN(64);  /* Match VGLITE_MEM_ALIGNMENT from vg_lite_hal.c */
    KEEP(*(.vglite.noinit.mem_pool))
    KEEP(*(.vglite.noinit.mem_pool.*))
    . = ALIGN(64);
} GROUP_DATA_LINK_IN(RAMABLE_REGION, ROMABLE_REGION)

