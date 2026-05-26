#include "flash_utils.h"
#include "board.h"

#include <cc.h>

__attribute__((section(".TI.ramfunc")))
Fapi_StatusType Configure_flashAPI(void) {

    Fapi_FlashBankType      bankType = C29Bank;
    Fapi_FOTAStatus         FOTAStatus = Active_Bank;
    Fapi_StatusType         oReturnCheck = Fapi_Error_Fail;
    Fapi_LibraryInfoType    libInfo;
    uint32_t                u32UserFlashConfig = 0;

    
    // Call Flash Initialization to setup flash waitstates.
    Flash_initModule(3);

    // Request semaphore for CPU1
    if ( !SSU_claimFlashSemaphore() ) {
        return Fapi_Error_Fail;
    }

    // Get the current FOTA configuration. This variable is passed to
    // as a parameter to most functions and if in Mode 1 or 3, determines
    // whether or not the update region will be written to
    u32UserFlashConfig = Fapi_getUserConfiguration(bankType, FOTAStatus);

    // Set Flash API Bank Mode configuration. These address mappings
    // will be used for program and erase operations. Outside of specific
    // use cases, this should match the device's currently programmed bank mode
    oReturnCheck = Fapi_SetFlashCPUConfiguration(Mode0);
    if(oReturnCheck != Fapi_Status_Success) {
        return oReturnCheck;
    }

    // Initialize the Flash API by providing the Flash register base address
    // and operating frequency(in MHz).
    // This function is required to initialize the Flash API based on System
    // frequency before any other Flash API operation can be performed.
    // This function must also be called whenever System frequency or RWAIT is changed.
    oReturnCheck = Fapi_initializeAPI((Fapi_FmcRegistersType*) FLASHCONTROLLER1_BASE, 200);
    if(oReturnCheck != Fapi_Status_Success) {
        return oReturnCheck;
    }

    // Flash API functions should not be executed from the same bank on which
    // erase/program operations are in progress.
    // Also, note that there should not be any access to the Flash bank on
    // which erase/program operations are in progress.
    //Example_CallFlashAPI();

    libInfo = Fapi_getLibraryInfo();
    printf("Flash lib info %d.%d %d\n",
        libInfo.u8ApiMajorVersion,
        libInfo.u8ApiMinorVersion,
        libInfo.u8ApiRevision);

    // Release the flash semaphore
    if (!SSU_releaseFlashSemaphore()) {
        return Fapi_Error_Fail;
    }

    return Fapi_Status_Success;

}


__attribute__((section(".TI.ramfunc")))
Fapi_StatusType Erase_flash_sector(uint32_t address) {

    return Fapi_Status_Success;
}


__attribute__((section(".TI.ramfunc")))
bool Write_flash(uint32_t flash_addr, void * src, uint32_t size) {

    SSU_claimFlashSemaphore();

    SSU_releaseFlashSemaphore();

    return 1;
}