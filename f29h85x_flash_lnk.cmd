--entry_point="code_start"

#define APP1_ENTRY 0x10040000

MEMORY
{
    SRAM_LDAx        : o=0x200E0000, l=0x1FFF8
    BOOT_REQUEST     : o=0x200FFFF8, l=0x00008
    SRAM_LPAx        : o=0x20100000, l=0x10000
    SRAM_CPAx        : o=0x20110000, l=0x10000
    SRAM_CDAx        : o=0x20120000, l=0x30000

    /* 0x10000000..0x1003FFFF belongs to bl-f29h85x-ecat. The final two
     * 4-KiB sectors of this slot are reserved for manifest and commit data. */
    CPU1_FLASH_APP   : o=APP1_ENTRY, l=0x1BE000

    FLASH_PARAMS     : o=0x10C00000, l=0x00800
    FLASH_CALIB      : o=0x10C00800, l=0x00800
}

SECTIONS
{
    codestart     : {} > APP1_ENTRY,       palign(8)
    cert          : {} > 0x0,              type=DSECT

    .text         : {} > CPU1_FLASH_APP,   palign(8)
    .binit        : {} > CPU1_FLASH_APP,   palign(8)
    .cinit        : {} > CPU1_FLASH_APP,   palign(8)
    .const        : {} > CPU1_FLASH_APP,   palign(8)
    .rodata       : {} > CPU1_FLASH_APP,   palign(8)
    .init_array   : {} > CPU1_FLASH_APP,   palign(8)
    .TI.ramfunc   : { -l F29H85x_NWFlashAPI_v21.00.00.00.lib }
                    LOAD=CPU1_FLASH_APP, RUN=SRAM_LPAx,
                    table(BINIT), palign(8)

    .data         : {} > SRAM_LDAx
    .bss          : {} > SRAM_LDAx
    .stack        : {} > SRAM_LDAx
    .sysmem       : {} > SRAM_LDAx
    .cio          : {} > SRAM_LDAx
    .boot_request : {} > BOOT_REQUEST, type=NOINIT

    .params       : {} > FLASH_PARAMS, palign(8), type=NOLOAD
    .calib        : {} > FLASH_CALIB,  palign(8), type=NOLOAD
}
