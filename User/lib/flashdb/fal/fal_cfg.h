/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-05-17     armink       the first version
 */

#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_
#include "eeprom_m95.h"
#include "eeprom_fm24.h"

#define FAL_PART_HAS_TABLE_CFG

#define EEPROM_M95_1_BLOCK_SIZE M95_PAGE_SIZE_128 * 64
#define EEPROM_M95_2_BLOCK_SIZE M95_PAGE_SIZE_256 * 32
#define EEPROM_FM24_BLOCK_SIZE FM24_PAGE_SIZE * 8

#define EEPROM_M95_1_SIZE _M95512_
#define EEPROM_M95_2_SIZE _M95M02_
#define EEPROM_FM24_SIZE FM24_SIZE

#define EEPROM_M95_1_DEV_NAME "eeprom_m95_1"
#define EEPROM_M95_2_DEV_NAME "eeprom_m95_2"
#define EEPROM_FM24_DEV_NAME "eeprom_fm24"

/* ===================== Flash device Configuration ========================= */
extern struct fal_flash_dev eeprom_m95_1;
extern struct fal_flash_dev eeprom_m95_2;
extern struct fal_flash_dev eeprom_fm24;

/* flash device table */
#define FAL_FLASH_DEV_TABLE \
    {                       \
        &eeprom_m95_1,      \
            &eeprom_m95_2,  \
            &eeprom_fm24,   \
    }

/* ====================== Partition Configuration ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG
/* partition table */
// issues :https://github.com/armink/FlashDB/issues/40  ;epprom的扇区太小
#define FAL_PART_TABLE                                                                     \
    {                                                                                      \
        {FAL_PART_MAGIC_WORD, "KVDB", EEPROM_M95_1_DEV_NAME, 0, EEPROM_M95_1_SIZE, 0},     \
            {FAL_PART_MAGIC_WORD, "TSDB", EEPROM_M95_2_DEV_NAME, 0, EEPROM_M95_2_SIZE, 0}, \
            {FAL_PART_MAGIC_WORD, "RTDB", EEPROM_FM24_DEV_NAME, 0, EEPROM_FM24_SIZE, 0},   \
    }

#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _FAL_CFG_H_ */
