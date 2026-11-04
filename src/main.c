//
// Included Files
//

#include "ethercat_subdevice_cpu1_hal.h"
#include "board.h"
#include "stdio.h"

#include <cc.h>
#include <soes/ecat_slv.h>
#include "flash_utils.h"
#include "globals.h"
#include "params.h"

#define LOG_BUFF_SIZE  128 
static uint8_t log_buff[LOG_BUFF_SIZE];

extern esc_cfg_t config;

#if 0
// redirect printf to ccs CIO ... Properties/Debug/Enable CIO function use 
int _write(int file, char *ptr, int len) {
    UART_writeCharArray(dbg_UART0_BASE, (uint8_t*)ptr, len);
    return 0;
}
#endif

void dbg_printf(const char *fmt, ...)
{
    va_list args;
    
    va_start(args, fmt);
    vsnprintf((char *)(log_buff), LOG_BUFF_SIZE, fmt, args);
    UART_writeCharArray(dbg_UART0_BASE, log_buff, strlen(log_buff));
    va_end(args);
}

__attribute__((interrupt ))
void INT_myCPUTIMER0_ISR(void) {

    GPIO_writePin(dbg_1, 1);
    if ( ! ESC_SYNCactivation() ) {
		ecat_slv();
	}
    GPIO_writePin(dbg_1, 0);
}

void Sync0_Isr(void) {

    GPIO_writePin(dbg_2, 1);
    ecat_slv();
	GPIO_writePin(dbg_2, 0);
}

//
// Main
//
int main()
{
    uint16_t initStatus;

    // Initialize CPU1 and HAL interface
    initStatus = ESC_initHW();
    // Initialize SysConfig Settings
    Board_init();
    print_build_info();

    // Loop and signal error if initHW returns failure
    if(initStatus == ESC_HW_INIT_FAIL)
    {
        printf("FAIL ESC_initHW\n");
    }
    
    //
    if ( Configure_flashAPI() != Fapi_Status_Success ) {
        printf("FAIL Configure_flashAPI\n");
    }
    if (Read_Flash_Params() == PARAMS_CMD_ERROR) {
        //
        //glob_fault.bit.warn_read_flash = 1;
        DPRINT("Read_Flash_Params FAIL\n");
        if (Load_Default_Params() == PARAMS_CMD_ERROR) {
            // FATAL ERROR
            //Error_Handler();
        }
        DPRINT("Load_Default_Params\n");
    }

    //if ( Erase_dataFlashSector((uint32_t)&flash_sdo, sizeof(flash_sdo)) != Fapi_Status_Success ) {
    //    DPRINT("FAIL erase data sector !!!\n");
    //}

    DPRINT("sdo.ram.fw_ver=%s\n", sdo.ram.fw_ver);
    DPRINT("FLASH_SDO\n");
    print_sdo(&flash_sdo);
    DPRINT("DFLT_FLASH_SDO\n");
    print_sdo(&dflt_flash_sdo);
    DPRINT("SDO\n");
    print_sdo(&sdo.flash);
    
    // Initialize SOES
    ecat_slv_init(&config);

    // start timer 
    CPUTimer_startTimer(myCPUTIMER0_BASE);
    
    // Update local RAM with ESC register values for debugging
    while(1)
    {       
        GPIO_togglePin(DEVICE_GPIO_PIN_LED1);
        DEVICE_DELAY_US((uint32_t)(500000));
    }

    return 0;
}

//
// End of File
//
