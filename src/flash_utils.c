#include "flash_utils.h"
#include "board.h"

#include <stdio.h>

void Disable_interrupt()	{ DINT; }
void Enable_interrupt()		{ EINT; }

uint32_t u32UserFlashConfig = 0;
static bool flashApiConfigured = false;

static inline __attribute__((always_inline))
bool IsDataFlashRangeValid(uint32_t address, uint32_t size) {
    return (size != 0U) &&
           (address >= C29FLASHBANKFR4RP0STARTADDRESS) &&
           (address < C29FLASHBANKFR4RP0ENDADDRESSPLUS1) &&
           (size <= (C29FLASHBANKFR4RP0ENDADDRESSPLUS1 - address));
}

static inline __attribute__((always_inline))
Fapi_StatusType ReleaseFlashSemaphore(Fapi_StatusType status)
{
    if (!SSU_releaseFlashSemaphore()) {
        return Fapi_Error_Fail;
    }

    return status;
}

static __attribute__((section(".TI.ramfunc")))
Fapi_StatusType ClearFSMStatus(uint32_t u32StartAddress, uint32_t userFlashConfig)
{
    Fapi_FlashStatusType oFlashStatus;
    Fapi_StatusType oReturnCheck;

    while (Fapi_checkFsmForReady(u32StartAddress, userFlashConfig) != Fapi_Status_FsmReady) {}

    oFlashStatus = Fapi_getFsmStatus(u32StartAddress, userFlashConfig);
    if (oFlashStatus != 0U) {
        oReturnCheck = Fapi_issueAsyncCommand(u32StartAddress, userFlashConfig,
                                             Fapi_ClearStatus);
        while (Fapi_getFsmStatus(u32StartAddress, userFlashConfig) != 0U) {}

        if (oReturnCheck != Fapi_Status_Success) {
            return oReturnCheck;
        }
    }

    return Fapi_Status_Success;
}


__attribute__((section(".TI.ramfunc")))
Fapi_StatusType Configure_flashAPI(void)
{
    Fapi_FlashBankType bankType = C29Bank;
    Fapi_FOTAStatus FOTAStatus = Active_Bank;
    Fapi_StatusType oReturnCheck;
    Fapi_LibraryInfoType libInfo;

    flashApiConfigured = false;
    Flash_initModule(3);

    if (!SSU_claimFlashSemaphore()) {
        return Fapi_Error_Fail;
    }

    // Store the configuration in the global used by all later API calls.
    u32UserFlashConfig = Fapi_getUserConfiguration(bankType, FOTAStatus);

    /* The application spans RP0 and RP1, matching the bootloader's Mode1
     * active-bank layout. */
    oReturnCheck = Fapi_SetFlashCPUConfiguration(Mode1);
    if (oReturnCheck != Fapi_Status_Success) {
        return ReleaseFlashSemaphore(oReturnCheck);
    }

    oReturnCheck = Fapi_initializeAPI((Fapi_FmcRegistersType *)FLASHCONTROLLER1_BASE,
                                      200U);
    if (oReturnCheck != Fapi_Status_Success) {
        return ReleaseFlashSemaphore(oReturnCheck);
    }

    libInfo = Fapi_getLibraryInfo();
    printf("Flash lib info %d.%d %d\n",
           libInfo.u8ApiMajorVersion,
           libInfo.u8ApiMinorVersion,
           libInfo.u8ApiRevision);

    if (!SSU_releaseFlashSemaphore()) {
        return Fapi_Error_Fail;
    }

    flashApiConfigured = true;
    return Fapi_Status_Success;
}


__attribute__((section(".TI.ramfunc")))
Fapi_StatusType Erase_dataFlashSector(uint32_t address, uint32_t size)
{
    Fapi_StatusType oReturnCheck;
    Fapi_FlashStatusType oFlashStatus;
    Fapi_FlashStatusWordType oFlashStatusWord;
    const uint8_t u8Iterator = 0U;
    uint32_t u32EndAddress;

    if (!flashApiConfigured ||
        !IsDataFlashRangeValid(address, size) ||
        ((address % DATA_FLASH_SECTOR_SIZE_BYTES) != 0U))
    {
        return Fapi_Error_InvalidAddress;
    }

    // Flash erase operates on complete 2 KB sectors. Round the requested
    // byte count up so a final partially covered sector is erased as well.
    const uint32_t sectorCount =
        (size + DATA_FLASH_SECTOR_SIZE_BYTES - 1U) /
        DATA_FLASH_SECTOR_SIZE_BYTES;
    u32EndAddress = address + (sectorCount * DATA_FLASH_SECTOR_SIZE_BYTES);

    if (!SSU_claimFlashSemaphore()) {
        return Fapi_Error_Fail;
    }

    for (uint32_t u32CurrentAddress = address;
         u32CurrentAddress < u32EndAddress;
         u32CurrentAddress += DATA_FLASH_SECTOR_SIZE_BYTES)
    {
        oReturnCheck = ClearFSMStatus(u32CurrentAddress, u32UserFlashConfig);
        if (oReturnCheck != Fapi_Status_Success) {
            return ReleaseFlashSemaphore(oReturnCheck);
        }

        Fapi_setupBankSectorEnable((uint32_t *)u32CurrentAddress, u32UserFlashConfig,
                                   FLASH_NOWRAPPER_O_CMDWEPROTA, 0x00000000U);
        Fapi_setupBankSectorEnable((uint32_t *)u32CurrentAddress, u32UserFlashConfig,
                                   FLASH_NOWRAPPER_O_CMDWEPROTB, 0x00000000U);

        oReturnCheck = Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector,
                                                         (uint32_t *)u32CurrentAddress,
                                                         u8Iterator,
                                                         u32UserFlashConfig);
        while (Fapi_checkFsmForReady(u32CurrentAddress, u32UserFlashConfig) ==
               Fapi_Status_FsmBusy) {}

        if (oReturnCheck != Fapi_Status_Success) {
            return ReleaseFlashSemaphore(oReturnCheck);
        }

        oFlashStatus = Fapi_getFsmStatus(u32CurrentAddress, u32UserFlashConfig);
        if (oFlashStatus != 3U) {
            return ReleaseFlashSemaphore(Fapi_Error_Fail);
        }

        oReturnCheck = Fapi_doBlankCheck((uint32_t *)u32CurrentAddress,
                                         DataBankSector2KB_u32length,
                                         &oFlashStatusWord,
                                         u8Iterator,
                                         u32UserFlashConfig);
        if (oReturnCheck != Fapi_Status_Success) {
            return ReleaseFlashSemaphore(oReturnCheck);
        }
    }

    return ReleaseFlashSemaphore(Fapi_Status_Success);
}

__attribute__((section(".TI.ramfunc")))
Fapi_StatusType Program_dataFlashSector(const uint8_t *src, uint32_t address, uint32_t size)
{
    Fapi_StatusType oReturnCheck;
    Fapi_FlashStatusType oFlashStatus;
    Fapi_FlashStatusWordType oFlashStatusWord;
    uint8_t programData[DATA_FLASH_PROGRAM_SIZE_BYTES];

    if (!flashApiConfigured ||
        (src == NULL) ||
        !IsDataFlashRangeValid(address, size) ||
        ((address % DATA_FLASH_PROGRAM_SIZE_BYTES) != 0U))
    {
        return Fapi_Error_InvalidAddress;
    }

    if (!SSU_claimFlashSemaphore()) {
        return Fapi_Error_Fail;
    }

    for (uint32_t offset = 0U; offset < size; offset += DATA_FLASH_PROGRAM_SIZE_BYTES)
    {
        uint32_t chunkSize = size - offset;
        uint32_t u32Index = address + offset;

        if (chunkSize > DATA_FLASH_PROGRAM_SIZE_BYTES) {
            chunkSize = DATA_FLASH_PROGRAM_SIZE_BYTES;
        }

        // AutoECC programs a complete 128-bit unit. Padding prevents reading
        // beyond the caller's buffer for a final partial unit.
        for (uint32_t i = 0U; i < DATA_FLASH_PROGRAM_SIZE_BYTES; i++) {
            programData[i] = (i < chunkSize) ? src[offset + i] : 0xFFU;
        }

        oReturnCheck = ClearFSMStatus(u32Index, u32UserFlashConfig);
        if (oReturnCheck != Fapi_Status_Success) {
            return ReleaseFlashSemaphore(oReturnCheck);
        }

        Fapi_setupBankSectorEnable((uint32_t *)u32Index, u32UserFlashConfig,
                                   FLASH_NOWRAPPER_O_CMDWEPROTA, 0x00000000U);
        Fapi_setupBankSectorEnable((uint32_t *)u32Index, u32UserFlashConfig,
                                   FLASH_NOWRAPPER_O_CMDWEPROTB, 0x00000000U);

        oReturnCheck = Fapi_issueProgrammingCommand((uint32_t *)u32Index,
                                                    programData,
                                                    DATA_FLASH_PROGRAM_SIZE_BYTES,
                                                    0, 0,
                                                    Fapi_AutoEccGeneration,
                                                    u32UserFlashConfig);
        while (Fapi_checkFsmForReady(u32Index, u32UserFlashConfig) ==
               Fapi_Status_FsmBusy) {}

        if (oReturnCheck != Fapi_Status_Success) {
            return ReleaseFlashSemaphore(oReturnCheck);
        }

        oFlashStatus = Fapi_getFsmStatus(u32Index, u32UserFlashConfig);
        if (oFlashStatus != 3U) {
            return ReleaseFlashSemaphore(Fapi_Error_Fail);
        }

        oReturnCheck = Fapi_doVerifyByByte((uint8_t *)u32Index,
                                           DATA_FLASH_PROGRAM_SIZE_BYTES,
                                           programData,
                                           &oFlashStatusWord,
                                           0U,
                                           u32UserFlashConfig);
        if (oReturnCheck != Fapi_Status_Success) {
            return ReleaseFlashSemaphore(oReturnCheck);
        }
    }

    return ReleaseFlashSemaphore(Fapi_Status_Success);
}

/**
    using data-only flash bank 256 kB divided in 128 sectors of 2kB 
    FLASH_RP4 : o=0x10C00000, l=0x40000
 */
__attribute__((section(".TI.ramfunc")))
bool Write_flash(uint32_t flash_addr, const void *src, uint32_t size) {
    
    // erase sector 
    if (Erase_dataFlashSector(flash_addr, size) != Fapi_Status_Success) {
        return false;
    }
    // program with src content
    if (Program_dataFlashSector(src, flash_addr, size) != Fapi_Status_Success) {
        return false;
    }
    return true;
}
