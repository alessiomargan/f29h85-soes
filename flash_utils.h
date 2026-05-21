#ifndef _FLASH_UTILS_H_
#define _FLASH_UTILS_H_

#include <flash.h>
#include <FlashTech_F29H85x_C29x.h>

#define FLASH_PARAM_ADDR    C29FLASHBANKFR1RP1STARTADDRESS

void Configure_flashAPI(void);

bool Write_flash(uint32_t flash_addr, void * src, uint32_t size);

#endif