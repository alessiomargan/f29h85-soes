#ifndef _FLASH_UTILS_H_
#define _FLASH_UTILS_H_

#include <flash.h>

void Configure_flashAPI(void);

bool Write_flash(uint32_t flash_addr, void * src, uint32_t size);

#endif