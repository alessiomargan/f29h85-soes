#ifndef _FLASH_UTILS_H_
#define _FLASH_UTILS_H_

#include "flash.h"
#include "FlashTech_F29H85x_C29x.h"

#define FLASH_PARAM_ADDR            C29FLASHBANKFR4RP0STARTADDRESS

//Length of region in 32-bit words to blank check 2KB * 8 / 32
#define DataBankSector2KB_u32length 0x200U

Fapi_StatusType Configure_flashAPI(void);
Fapi_StatusType Erase_dataFlashSector(uint32_t address, uint32_t size);
Fapi_StatusType Program_dataFlashSector(uint8_t *src, uint32_t address, uint32_t size);
bool Write_flash(uint32_t flash_addr, void * src, uint32_t size);

#endif