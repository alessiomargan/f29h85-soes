#include "ethercat_subdevice_cpu1_hal.h"

#include <stdint.h>

#define BOOT_REQUEST_MAGIC      0xB007B007UL
#define BOOT_LOADER_ENTRY_ADDR  0x10001000UL

/* This NOINIT object has the same fixed SRAM address in both linker files. */
__attribute__((section(".boot_request")))
static volatile struct {
    uint32_t magic;
    uint32_t magic_inverse;
} boot_request;

void jump_to_bootloader(void)
{
    void (*boot_entry)(void) = (void (*)(void))BOOT_LOADER_ENTRY_ADDR;

    DINT;
    boot_request.magic_inverse = ~BOOT_REQUEST_MAGIC;
    boot_request.magic = BOOT_REQUEST_MAGIC;

    /* Enter codestart so the bootloader rebuilds its C runtime and stack. */
    boot_entry();
    for (;;) {}
}
