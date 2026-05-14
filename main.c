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
extern uint32_t ESC_SYNCactivation(void);

// redirect printf to ccs CIO
int _write(int file, char *ptr, int len) {
    UART_writeCharArray(UARTA_BASE, (uint8_t*)ptr, len);
    return 0;
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

    Board_init();
    printf("Hello world !!\n");
    
    // Loop and signal error if initHW returns failure
    if(initStatus == ESC_HW_INIT_FAIL)
    {
        printf("FAIL ESC_initHW");
    }
    
    // Init soes
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
