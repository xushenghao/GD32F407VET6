#ifndef __BOOTLOAD_H
#define __BOOTLOAD_H
#include "lib.h"
#include "flash.h"
#include "flow.h"

/**
 * @brief Defines the start address of the application in flash memory.
 */
#define BOOTLOAD_APP_START_ADDRESS ((uint32_t)0x08000000u)

/**
 * @brief Defines the end address of the application in flash memory.
 */
#define BOOTLOAD_APP_END_ADDRESS (BOOTLOAD_APP_START_ADDRESS + 220 * LL_FLASH_PAGE_SIZE) // 220*2048 = 450560

/**
 * @brief Defines the start address of the bootloader in flash memory.
 */
#define BOOTLOAD_START_ADDRESS BOOTLOAD_APP_END_ADDRESS

/**
 * @brief Defines the start address of the backup area for the application in flash memory.
 */
#define BOOTLOAD_APP_BACKUP_ADDR_START (BOOTLOAD_APP_START_ADDRESS + LL_FLASH_BANK1_PAGE_NUM * LL_FLASH_PAGE_SIZE)

/**
 * @brief Defines the timeout for the bootloading process.
 *
 * This macro defines the timeout for the bootloading process, in seconds. The default value is 10 seconds.
 */
#define BOOTLOAD_TIMEOUT 10

/**
 * @brief A function pointer type for a bootload transmit callback function.
 *
 * This function pointer type is used for a bootload transmit callback function, which is
 * called when data needs to be transmitted to the bootloader. The function is passed the
 * source of the data (the index of the data packet), a pointer to the data buffer, and the
 * length of the data buffer.
 *
 * @param data_src The index of the data packet that is being transmitted.
 * @param buf A pointer to the data buffer.
 * @param len The length of the data buffer.
 */
typedef void (*bootload_transmit_callback)(const uint8_t data_src, const uint8_t *buf, const uint16_t len);

/**
 * @brief A function pointer type for a bootload end callback function.
 *
 * This function pointer type is used for a bootload end callback function, which is called
 * when the bootloading process is complete. The function takes no parameters and returns no
 * value.
 */
typedef void (*bootload_end_callback)(BOOL);

/**
 * @brief initializes the bootloader
 *
 * This function initializes the bootloader, including setting up the communication
 * with the host and configuring the flash memory for bootloading.
 *
 * @param transmit a pointer to the function that will be called to transmit data to the host
 */
void bootload_init(bootload_transmit_callback transmit, bootload_end_callback end);

/**
 * @brief Transmits data from the specified index.
 *
 * This function transmits data from the specified index to the bootloader.
 *
 * @param to_index The index from which the data should be transmitted.
 */
void bootload_transmit_from(const uint8_t to_index);

/**
 * @brief Jumps to the specified address.
 *
 * This function jumps to the specified address, which is typically the start address of the bootloader.
 *
 * @param address The address to jump to.
 */
void bootload_jump(uint32_t address);

/**
 * @brief Performs inspection of the bootloader.
 *
 * This function performs inspection of the bootloader, such as checking the version or integrity of the bootloader.
 */
void bootload_inspection(void);

/**
 * @brief Checks if a timeout has occurred.
 *
 * This function checks if a timeout has occurred during the bootloading process.
 *
 * @return TRUE if a timeout has occurred, FALSE otherwise.
 */
BOOL bootload_timeout(void);
#endif // __BOOTLOAD_H
