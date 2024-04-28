#include "bootload.h"
#include "ymodem.h"
#include "sys.h"
#include "delay.h"
#include "cmac.h"
#define AES_CMAC_DIGEST_LENGTH 16
typedef void (*fnc_ptr)(void); // 用于跳转到应用程序的函数指针
static bootload_transmit_callback transmit_callback = NULL;
static bootload_end_callback end_callback = NULL;
uint8_t upgrade_key[] =
    {
        0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
        0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C}; // 密钥

static AES_CMAC_CTX upgrade_ctx;                                   /**< AES CMAC context for upgrade process */
static uint8_t pre_upgrade_mic[AES_CMAC_DIGEST_LENGTH];            /**< MIC (Message Integrity Code) before upgrade */
static uint8_t upgrade_mic[AES_CMAC_DIGEST_LENGTH];                /**< MIC (Message Integrity Code) after upgrade */
static uint32_t flashdestination = BOOTLOAD_APP_BACKUP_ADDR_START; /**< Flash destination address for bootloading */
static uint32_t upgrade_size = 0;                                  /**< Size of the upgrade */
static uint8_t read_cache[LL_FLASH_PAGE_SIZE];                     /**< Read cache for flash pages */
static uint8_t data_src_from;                                      /**< Data source for bootloading */

/**
 * @brief Perform bootload inspection.
 *
 * This function calls the `rym_process()` function to perform bootload inspection.
 */
void bootload_inspection(void)
{
    rym_process();
}

/**
 * @brief Checks if the bootload timeout has occurred.
 *
 * This function calls the `rym_timeout()` function to determine if the bootload timeout has occurred.
 *
 * @return TRUE if the bootload timeout has occurred, FALSE otherwise.
 */
BOOL bootload_timeout(void)
{
    return rym_timeout();
}

/**
 * @brief Handles the beginning of the bootloading process.
 *
 * This function is responsible for handling the initial steps of the bootloading process.
 * It checks if the length of the data to be bootloaded is greater than the end address of the application area.
 * If the length is greater, it returns an error code indicating that the bootloading cannot proceed.
 * Otherwise, it sets the flash destination address to the start address of the backup area,
 * erases the bank of the flash memory where the application is stored,
 * and initializes the AES-CMAC context for the upgrade process.
 *
 * @param p Pointer to the data to be bootloaded.
 * @param len Length of the data to be bootloaded.
 * @return Error code indicating the result of the operation.
 *         - RYM_CODE_CAN: If the length of the data is greater than the end address of the application area.
 *         - RYM_CODE_NONE: If the operation is successful.
 */
static rym_code_e on_begin(uint8_t *p, uint32_t len)
{
    if (len > BOOTLOAD_APP_END_ADDRESS)
    {
        return RYM_CODE_CAN;
    }

    flashdestination = BOOTLOAD_APP_BACKUP_ADDR_START;
    LL_FLASH_Unlock(FLASH);
    LL_FLASH_EraseBank(LL_FLASH_BANK2);
    LL_FLASH_Lock(FLASH);

    AES_CMAC_Init(&upgrade_ctx);
    AES_CMAC_SetKey(&upgrade_ctx, upgrade_key);
    return RYM_CODE_NONE;
}

/**
 * @brief Handles the received data and programs it into the flash memory.
 *
 * This function unlocks the flash memory, programs the received data into the specified flash destination,
 * locks the flash memory again, updates the AES-CMAC context with the received data, and increments the flash destination.
 *
 * @param p Pointer to the data buffer.
 * @param len Length of the data buffer.
 * @return The result code indicating the success or failure of the operation.
 */
static rym_code_e on_data(uint8_t *p, uint32_t len)
{
    LL_FLASH_Unlock(FLASH);
    LL_FLASH_Program(flashdestination, p, len);
    LL_FLASH_Lock(FLASH);
    AES_CMAC_Update(&upgrade_ctx, p, len);
    flashdestination += len;
    return RYM_CODE_NONE;
}

/**
 * @brief Calculate the MIC (Message Integrity Code) for the firmware upgrade.
 *
 * This function calculates the MIC using AES-CMAC algorithm for the firmware upgrade data.
 * It reads the firmware data from flash memory in pages and updates the MIC calculation.
 * Finally, it compares the calculated MIC with the pre-upgrade MIC to determine the success of the upgrade.
 *
 * @param p Pointer to the firmware data.
 * @param len Length of the firmware data.
 * @return The result code indicating the success or failure of the upgrade process.
 */
static rym_code_e on_end(uint8_t *p, uint32_t len)
{
    AES_CMAC_Final(pre_upgrade_mic, &upgrade_ctx);
    upgrade_size = flashdestination - BOOTLOAD_APP_BACKUP_ADDR_START;

    AES_CMAC_Init(&upgrade_ctx);
    AES_CMAC_SetKey(&upgrade_ctx, upgrade_key);
    uint32_t start = BOOTLOAD_APP_BACKUP_ADDR_START;
    uint16_t num = (upgrade_size / LL_FLASH_PAGE_SIZE);
    uint16_t remain = (upgrade_size % LL_FLASH_PAGE_SIZE);

    // STM32L476RG的flash页大小为2K,先读取整数页，再读取余数
    for (uint16_t i = 0; i < num; i++)
    {
        LL_FLASH_Read((start + i * (LL_FLASH_PAGE_SIZE)), read_cache, LL_FLASH_PAGE_SIZE);
        AES_CMAC_Update(&upgrade_ctx, read_cache, LL_FLASH_PAGE_SIZE);
    }

    if (remain)
    {
        osel_memset(read_cache, 0, LL_FLASH_PAGE_SIZE);
        LL_FLASH_Read((start + num * (LL_FLASH_PAGE_SIZE)), read_cache, remain);
        AES_CMAC_Update(&upgrade_ctx, read_cache, remain);
    }

    AES_CMAC_Final(upgrade_mic, &upgrade_ctx);

    // 比较mic，相同可以写入标志位告知应用程序升级成功
    if (osel_memcmp(upgrade_mic, pre_upgrade_mic, AES_CMAC_DIGEST_LENGTH) == 0)
    {
        end_callback(TRUE);
    }
    else
    {
        end_callback(FALSE);
    }
    return RYM_CODE_NONE;
}

/**
 * @brief Handles the transmission of data.
 *
 * This function calls the transmit_callback function to transmit the data from the specified source.
 *
 * @param p Pointer to the data buffer.
 * @param len Length of the data buffer.
 * @return The return code indicating the status of the transmission.
 */
static rym_code_e on_transmit(uint8_t *p, uint32_t len)
{
    transmit_callback(data_src_from, p, (uint16_t)len);
    return RYM_CODE_NONE;
}

/**
 * @brief Initializes the bootload module.
 *
 * This function initializes the bootload module by setting the transmit callback
 * and configuring the RYM module. It asserts if the initialization fails.
 *
 * @param transmit The transmit callback function.
 */
void bootload_init(bootload_transmit_callback transmit, bootload_end_callback end)
{
    BOOL res = FALSE;

    transmit_callback = transmit;
    end_callback = end;

    res = rym_init();
    DBG_ASSERT(res == TRUE __DBG_LINE);

    res = rym_config(on_begin, on_data, on_end, on_transmit, BOOTLOAD_TIMEOUT);
    DBG_ASSERT(res == TRUE __DBG_LINE);
}

/**
 * @brief Sets the data source index for bootload transmission.
 *
 * This function sets the data source index for bootload transmission. The data source index
 * determines the starting point from which the data will be transmitted.
 *
 * @param to_index The index of the data source.
 */
void bootload_transmit_from(const uint8_t to_index)
{
    data_src_from = to_index;
}

/**
 * @brief Jump to the specified address and execute the bootloader.
 *
 * @param address The entry address of the bootloader.
 */
void bootload_jump(uint32_t address)
{
    // Get the entry address of the application program
    fnc_ptr jump_to_bootload;
    jump_to_bootload = (fnc_ptr)(*(__IO uint32_t *)(address + 4));

    // Disable RCC
    RCC->APB1ENR1 = 0;
    RCC->APB1ENR2 = 0;
    RCC->APB2ENR = 0;
    RCC->AHB1ENR = 0;
    RCC->AHB2ENR = 0;
    RCC->AHB3ENR = 0;

    // Disable SysTick
    SysTick->CTRL = 0;
    // 清空SysTick
    SysTick->LOAD = 0;
    // 清空SysTick
    SysTick->VAL = 0;
    // 设置向量表偏移地址
    SCB->VTOR = address;
    // 设置堆栈指针
    sys_msr_msp(*(__IO uint32_t *)address);
    // 跳转
    jump_to_bootload();
}
