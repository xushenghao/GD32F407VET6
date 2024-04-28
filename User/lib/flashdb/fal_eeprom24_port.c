/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-26     armink       the first version
 */
#include "eeprom_fm24.h"
#include <fal.h>

static int init(void);
static int erase(long offset, size_t size);

static int read(long offset, uint8_t *buf, size_t size);
static int write(long offset, const uint8_t *buf, size_t size);

// 1.定义 flash 设备
struct fal_flash_dev eeprom_fm24 =
    {
        .name = EEPROM_FM24_DEV_NAME,
        .addr = 0x000000,
        .len = EEPROM_FM24_SIZE,
        .blk_size = EEPROM_FM24_BLOCK_SIZE,
        .ops = {init, read, write, erase},
        .write_gran = 8};

static int init(void)
{
    return 1;
}

static int erase(long offset, size_t size)
{
    return size;
}

static int read(long offset, uint8_t *buf, size_t size)
{
    /* You can add your code under here. */
    uint32_t addr = eeprom_fm24.addr + offset;
    BOOL res = eeprom_fm24_read(addr, buf, size);
    return res == TRUE ? size : 0;
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    uint32_t addr = eeprom_fm24.addr + offset;
    eeprom_fm24_write(addr, (uint8_t *)buf, size);
    return size;
}
