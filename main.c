//
// Included Files
//

#include "ethercat_subdevice_cpu1_hal.h"
#include "board.h"
#include "stdio.h"

#include <soes/ecat_slv.h>
//#include <pins.h>
//#include <peripherals.h>

extern esc_cfg_t config;

// redirect printf to ccs CIO
int _write(int file, char *ptr, int len) {
    UART_writeCharArray(UARTA_BASE, (uint8_t*)ptr, len);
    return 0;
}



//
// Main
//
int main()
{
    uint32_t cnt;
    uint16_t initStatus;

    // Initialize CPU1 and HAL interface
    initStatus = ESC_initHW();

    Board_init();
    printf("Hello world !!\n");
    
    // Loop and signal error if initHW returns failure
    if(initStatus == ESC_HW_INIT_FAIL)
    {
        printf("FAIL ESC_initHW");
    }
    
    // Init soes
    ecat_slv_init(&config);

    // Update local RAM with ESC register values for debugging
    while(1)
    {
        ecat_slv();
        if ( ! (cnt++ % 1000) ) {
            GPIO_togglePin(DEVICE_GPIO_PIN_LED1);
        }
        DEVICE_DELAY_US((uint32_t)(500));
    }

    return 0;
}

//
// End of File
//
