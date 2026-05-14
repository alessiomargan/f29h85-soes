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

#ifndef BOARD_H
#define BOARD_H

#ifndef USE_PERIPHERAL_FRAMES
#define USE_PERIPHERAL_FRAMES
#endif

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

//
// Included Files
//

#include "driverlib.h"
#include "device.h"

//*****************************************************************************
//
// PinMux Configurations
//
//*****************************************************************************
//
// GPIO23 - GPIO Settings
//
#define myBoardLED1_GPIO_GPIO_PIN_CONFIG GPIO_23_GPIO23

//
// UARTA -> dbgUART0 Pinmux
//
//
// UARTA_TX - GPIO Settings
//
#define GPIO_PIN_UARTA_TX 42
#define dbgUART0_UARTTX_GPIO 42
#define dbgUART0_UARTTX_PIN_CONFIG GPIO_42_UARTA_TX
//
// UARTA_RX - GPIO Settings
//
#define GPIO_PIN_UARTA_RX 43
#define dbgUART0_UARTRX_GPIO 43
#define dbgUART0_UARTRX_PIN_CONFIG GPIO_43_UARTA_RX

//*****************************************************************************
//
// UART Configurations
//
//*****************************************************************************
//
// dbgUART0 Configuration
//
#define dbgUART0_BASE UARTA_BASE
#define dbgUART0_BAUDRATE 115200
#define dbgUART0_CONFIG_WLEN UART_CONFIG_WLEN_8
#define dbgUART0_CONFIG_STP2 UART_CONFIG_STOP_ONE
#define dbgUART0_CONFIG_EPS UART_CONFIG_PAR_ODD
#define dbgUART0_CONFIG_SPS 0
void    dbgUART0_init();




//*****************************************************************************
//
// RTDMA1 MPU Configurations
//
//*****************************************************************************


//*****************************************************************************
//
// RTDMA2 MPU Configurations
//
//*****************************************************************************


//*****************************************************************************
//
// CMPSS Configurations
//
//*****************************************************************************
void CMPSS_MUX_config();

//*****************************************************************************
//
// RTDMA1 Configurations
//
//*****************************************************************************
void RTDMA1_global_init();

//*****************************************************************************
//
// RTDMA2 Configurations
//
//*****************************************************************************
void RTDMA2_global_init();

//*****************************************************************************
//
// GPIO Configurations
//
//*****************************************************************************
#define myBoardLED1_GPIO 23
void myBoardLED1_GPIO_init();


//*****************************************************************************
//
// Board Configurations
//
//*****************************************************************************
void	Board_init();
void	UART_init();
void	PinMux_init();
void	PERCONFIG_init();
void	RTDMA1_MPU_init();
void	RTDMA2_MPU_init();
void    INPUTXBAR_init();
void	GPIO_init();
void	AIO_init();
void    CMPSS_init();
void	RTDMA1_init();
void	RTDMA2_init();

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif  // end of BOARD_H definition
