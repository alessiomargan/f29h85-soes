#include "flash_utils.h"
#include "board.h"

#include <cc.h>

void Disable_interrupt()	{ DINT; }
void Enable_interrupt()		{ EINT; }

uint32_t u32UserFlashConfig = 0;

inline static void FMSTAT_Fail(void)
{
    SSU_releaseFlashSemaphore();
    //asm(" EMUSTOP0");
    for (;;);
}

inline static void FlashAPI_Fail(Fapi_StatusType status)
{
    SSU_releaseFlashSemaphore();
    //asm(" EMUSTOP0");
    for (;;);
}

__attribute__((section(".TI.ramfunc")))
void ClearFSMStatus(uint32_t u32StartAddress, uint32_t u32UserFlashConfig) {

    Fapi_FlashStatusType  oFlashStatus;
    Fapi_StatusType  oReturnCheck;

    // Wait until FSM is done with the previous flash operation
    while (Fapi_checkFsmForReady(u32StartAddress, u32UserFlashConfig) != Fapi_Status_FsmReady);

    oFlashStatus = Fapi_getFsmStatus(u32StartAddress, u32UserFlashConfig);
    // If the status register is nonzero, then clear it
    if (oFlashStatus != 0)
    {
        // Clear the status register
        oReturnCheck = Fapi_issueAsyncCommand(u32StartAddress, u32UserFlashConfig, Fapi_ClearStatus);
        // Wait until status is cleared
        while (Fapi_getFsmStatus(u32StartAddress, u32UserFlashConfig) != 0) {}
        if(oReturnCheck != Fapi_Status_Success)
        {
            // Check Flash API documentation for possible errors
            FlashAPI_Fail(oReturnCheck);
        }
    }
}


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

    // Initialize the Flash API by providing the Flash register base address
    // and operating frequency(in MHz).
    // This function is required to initialize the Flash API based on System
    // frequency before any other Flash API operation can be performed.
    // This function must also be called whenever System frequency or RWAIT is changed.
    oReturnCheck = Fapi_initializeAPI((Fapi_FmcRegistersType*) FLASHCONTROLLER1_BASE, 200);
    if(oReturnCheck != Fapi_Status_Success) {
        return oReturnCheck;
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
Fapi_StatusType Erase_dataFlashSector(uint32_t address, uint32_t size) {

    Fapi_StatusType             oReturnCheck;
    Fapi_FlashStatusType        oFlashStatus;
    Fapi_FlashStatusWordType    oFlashStatusWord;
    
    // iterator for program and erase operations on interleaved banks.
    // 0: Data Flash/non-interleaved
    // 1: B0 or B2 (dependent on provided address)
    // 2: B1 or B3 (dependent on provided address)
    uint8_t     u8iterator = 0;
    // 128-bit aligned start address
    uint32_t    u32CurrentAddress;
    uint32_t    u32ErasedSize = 0;
    
    // Request semaphore for CPU1
    if ( !SSU_claimFlashSemaphore() ) {
        return Fapi_Error_Fail;
    }

    for (u32CurrentAddress = address;
         u32CurrentAddress < C29FLASHBANKFR4RP0ENDADDRESSPLUS1 && u32ErasedSize < size;
         u32CurrentAddress += 0x800, u32ErasedSize += 0x800)
    {

        ClearFSMStatus(u32CurrentAddress, u32UserFlashConfig);
        
        // Set program/erase protection
        Fapi_setupBankSectorEnable((uint32_t *)(u32CurrentAddress), u32UserFlashConfig,
                                    FLASH_NOWRAPPER_O_CMDWEPROTA, 0x00000000);
        Fapi_setupBankSectorEnable((uint32_t *)(u32CurrentAddress), u32UserFlashConfig,
                                    FLASH_NOWRAPPER_O_CMDWEPROTB, 0x00000000);

        // issues an erase cmd to the Flash State Machine along with a sector address
        oReturnCheck = Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector,
                                                        (uint32_t *)u32CurrentAddress,
                                                        u8iterator,
                                                        u32UserFlashConfig);
        // Wait until FSM is done with erase sector operation
        while (Fapi_checkFsmForReady(u32CurrentAddress, u32UserFlashConfig) == Fapi_Status_FsmBusy);

        if (oReturnCheck != Fapi_Status_Success) {
            // Check Flash API documentation for possible errors
            FlashAPI_Fail(oReturnCheck);
        }

        // Read FMSTAT register contents to know the status of FSM after
        // erase command to see if there are any erase operation related errors
        oFlashStatus = Fapi_getFsmStatus(u32CurrentAddress, u32UserFlashConfig);
        if (oFlashStatus != 3) {
            // Check Flash API documentation for FMSTAT and debug accordingly
            // Fapi_getFsmStatus() function gives the FMSTAT register contents.
            // Check to see if any of the EV bit, ESUSP bit, CSTAT bit or
            // VOLTSTAT bit is set (Refer to API documentation for more details).
            FMSTAT_Fail();
        }
        
        // Verify the Sector is erased
        oReturnCheck = Fapi_doBlankCheck((uint32_t *)(u32CurrentAddress),
                                        DataBankSector2KB_u32length,
                                        &oFlashStatusWord,
                                        u8iterator,
                                        u32UserFlashConfig);

        if(oReturnCheck != Fapi_Status_Success) {
            // Check Flash API documentation for error info
            FlashAPI_Fail(oReturnCheck);
        }
    
    } // end for

    // Release the flash semaphore
    if (!SSU_releaseFlashSemaphore()) {
        return Fapi_Error_Fail;
    }
    
    return Fapi_Status_Success;
}

__attribute__((section(".TI.ramfunc")))
Fapi_StatusType Program_dataFlashSector(uint8_t *src, uint32_t address, uint32_t size)
{
    Fapi_StatusType  oReturnCheck;
    Fapi_FlashStatusType  oFlashStatus;
    Fapi_FlashStatusWordType  oFlashStatusWord;

    // 128-bit aligned start address
    uint32_t    u32CurrentAddress;
    uint32_t    u32ProgSize = 0;
    
    // Request semaphore for CPU1
    if ( !SSU_claimFlashSemaphore() ) {
        return Fapi_Error_Fail;
    }

    for (u32CurrentAddress = address;
         u32CurrentAddress < C29FLASHBANKFR4RP0ENDADDRESSPLUS1 && u32ProgSize < size;
         u32CurrentAddress += 0x800, u32ProgSize += 0x800)
    {

        for(uint32_t i = 0, u32Index = u32CurrentAddress;
            u32Index < (u32CurrentAddress + size);
            i += 16, u32Index += 16)
        {

            ClearFSMStatus(u32Index, u32UserFlashConfig);

            // Set program/erase protection
            Fapi_setupBankSectorEnable((uint32_t *) u32Index, u32UserFlashConfig,
                                        FLASH_NOWRAPPER_O_CMDWEPROTA, 0x00000000);
            Fapi_setupBankSectorEnable((uint32_t *) u32Index, u32UserFlashConfig,
                                        FLASH_NOWRAPPER_O_CMDWEPROTB, 0x00000000);


            oReturnCheck = Fapi_issueProgrammingCommand((uint32_t *)u32Index,src+i,
                                                        16, 0, 0, Fapi_AutoEccGeneration,
                                                        u32UserFlashConfig);

            // Wait until the Flash program operation is over
            while(Fapi_checkFsmForReady(u32Index, u32UserFlashConfig) == Fapi_Status_FsmBusy);

            if(oReturnCheck != Fapi_Status_Success) {
                // Check Flash API documentation for possible errors
                FlashAPI_Fail(oReturnCheck);
            }

            // Read FMSTAT register contents to know the status of FSM after
            // program command to see if there are any program operation related errors
            oFlashStatus = Fapi_getFsmStatus(u32Index, u32UserFlashConfig);
            if(oFlashStatus != 3)
            {
                // Check FMSTAT and debug accordingly
                FMSTAT_Fail();
            }

            // Verify the programmed values.  Check for any ECC errors.
            oReturnCheck = Fapi_doVerifyByByte((uint8_t *) u32Index,
                                        16, (uint8_t *) (src + i),
                                        &oFlashStatusWord, 0, u32UserFlashConfig);

            if(oReturnCheck != Fapi_Status_Success) {
                // Check Flash API documentation for possible errors
                FlashAPI_Fail(oReturnCheck);
            }
        }
    }

    // Release the flash semaphore
    if (!SSU_releaseFlashSemaphore()) {
        return Fapi_Error_Fail;
    }

    return Fapi_Status_Success;
}

/**
    using data-only flash bank 256 kB divided in 128 sectors of 2kB 
    FLASH_RP4 : o=0x10C00000, l=0x40000
 */
__attribute__((section(".TI.ramfunc")))
bool Write_flash(uint32_t flash_addr, void * src, uint32_t size) {
    
    // erase sector 
    if (Erase_dataFlashSector(flash_addr, size) != Fapi_Status_Success) {
        return false;
    }
    if (Program_dataFlashSector(src, flash_addr, size) != Fapi_Status_Success) {
        return false;
    }
    return true;
}