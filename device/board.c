/*
 * Copyright (c) 2020 Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "board.h"

//*****************************************************************************
//
// Board Configurations
// Initializes the rest of the modules.
// Call this function in your application if you wish to do all module
// initialization.
// If you wish to not use some of the initializations, instead of the
// Board_init use the individual Module_inits
//
//*****************************************************************************
void Board_init()
{
	PinMux_init();
	PERCONFIG_init();
	INPUTXBAR_init();
	AIO_init();
	GPIO_init();
	CMPSS_init();
	RTDMA1_init();
	RTDMA2_init();
	UART_init();
}

//*****************************************************************************
//
// PINMUX Configurations
//
//*****************************************************************************
void PinMux_init()
{
	//
	// PinMux for modules assigned to CPU1
	//


	// GPIO23 -> myBoardLED1_GPIO Pinmux
	GPIO_setPinConfig(GPIO_23_GPIO23);
	//
	// UARTA -> dbgUART0 Pinmux
	//
	GPIO_setPinConfig(dbgUART0_UARTTX_PIN_CONFIG);
	GPIO_setPadConfig(dbgUART0_UARTTX_GPIO, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(dbgUART0_UARTTX_GPIO, GPIO_QUAL_ASYNC);

	GPIO_setPinConfig(dbgUART0_UARTRX_PIN_CONFIG);
	GPIO_setPadConfig(dbgUART0_UARTRX_GPIO, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(dbgUART0_UARTRX_GPIO, GPIO_QUAL_ASYNC);


}

//*****************************************************************************
//
// Peripheral Configurations
//
//*****************************************************************************
void PERCONFIG_init()
{
	//
	// Configuration for modules assigned to CPU1
	//
    SysCtl_selectCPUForPeripheral(SYSCTL_PERIPH_CONFIG_UARTA, SYSCTL_CPUSEL_CPU1);
    SysCtl_selectFrameForPeripheral(SYSCTL_PERIPH_CONFIG_UARTA, SYSCTL_FRAMESEL_0);

	//
	// Inserting some delay for frame updates to take effect
	//
    SysCtl_delay(100);
}


//*****************************************************************************
//
// INPUTXBAR Configurations
//
//*****************************************************************************
void INPUTXBAR_init(){
}



//*****************************************************************************
//
// RTDMA1 MPU Configurations
//
//*****************************************************************************
void RTDMA1_MPU_init(){
}

//*****************************************************************************
//
// RTDMA2 MPU Configurations
//
//*****************************************************************************
void RTDMA2_MPU_init(){
}

//*****************************************************************************
//
// CMPSS Configurations
//
//*****************************************************************************
void CMPSS_init(){
    CMPSS_MUX_config();
}

void CMPSS_MUX_config(){
}


//*****************************************************************************
//
// RTDMA1 Configurations
//
//*****************************************************************************
void RTDMA1_init(){
    RTDMA1_global_init();
    RTDMA1_MPU_init();
}

void RTDMA1_global_init(){
}


//*****************************************************************************
//
// RTDMA2 Configurations
//
//*****************************************************************************
void RTDMA2_init(){
    RTDMA2_global_init();
    RTDMA2_MPU_init();
}

void RTDMA2_global_init(){
}

//*****************************************************************************
//
// GPIO Configurations
//
//*****************************************************************************
void GPIO_init(){
	myBoardLED1_GPIO_init();
}

void myBoardLED1_GPIO_init(){
	GPIO_setPadConfig(myBoardLED1_GPIO, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(myBoardLED1_GPIO, GPIO_QUAL_SYNC);
	GPIO_setControllerCore(myBoardLED1_GPIO, GPIO_CORE_CPU1);
	GPIO_setDirectionMode(myBoardLED1_GPIO, GPIO_DIR_MODE_OUT);
}

//*****************************************************************************
//
// AIO Configurations
//
//*****************************************************************************
void AIO_init(){
}



//*****************************************************************************
//
// UART Configurations
//
//*****************************************************************************
void UART_init(){
    dbgUART0_init();
}

void dbgUART0_init(){
        //
        // Set dbgUART0 baud rate and configuration
        //
        UART_setConfig(
            dbgUART0_BASE, // base address
            DEVICE_SYSCLK_FREQ, // UART source clock
            115200, // baud rate
            (UART_CONFIG_WLEN_8 | // word length
             UART_CONFIG_STOP_ONE) // stop bits
        );
        
        //
        // FIFO disable
        //
        UART_disableFIFO(dbgUART0_BASE);
        
        //
        // Configure interrupts
        //
        UART_clearInterruptStatus(dbgUART0_BASE, 0xFFFF);
        UART_enableInterrupt(dbgUART0_BASE,
            (UART_INT_RX |
             UART_INT_TX)
        );
            
        UART_disableLoopback(dbgUART0_BASE);
        
        //
        // Enable RX, TX, and the UART.
        //
        UART_enableModuleNonFIFO(dbgUART0_BASE);
}

