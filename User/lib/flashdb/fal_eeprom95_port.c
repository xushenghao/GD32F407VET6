/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-26     armink       the first version
 */
#include "eeprom_m95.h"
#include <fal.h>

static int init(void);
static int erase(long offset, size_t size);

static int read1(long offset, uint8_t *buf, size_t size);
static int write1(long offset, const uint8_t *buf, size_t size);

static int read2(long offset, uint8_t *buf, size_t size);
static int write2(long offset, const uint8_t *buf, size_t size);

// 1.定义 flash 设备
struct fal_flash_dev eeprom_m95_1 =
    {
        .name = EEPROM_M95_1_DEV_NAME,
        .addr = 0x000000,
        .len = EEPROM_M95_1_SIZE,
        .blk_size = EEPROM_M95_1_BLOCK_SIZE,
        .ops = {init, read1, write1, erase},
        .write_gran = 8}; // 设置写粒度，单位 bit， 0 表示未生效（默认值为 0 ），该成员是 fal 版本大于 0.4.0 的新增成员。各个 flash 写入粒度不尽相同，可通过该成员进行设置，以下列举几种常见 Flash 写粒度

struct fal_flash_dev eeprom_m95_2 =
    {
        .name = EEPROM_M95_2_DEV_NAME,
        .addr = 0x000000,
        .len = EEPROM_M95_2_SIZE,
        .blk_size = EEPROM_M95_2_BLOCK_SIZE,
        .ops = {init, read2, write2, erase},
        .write_gran = 8};

static int init(void)
{
    return 1;
}

static int erase(long offset, size_t size)
{
    __NOP();
    return size;
}

static int read1(long offset, uint8_t *buf, size_t size)
{
    /* You can add your code under here. */
    uint32_t addr = eeprom_m95_1.addr + offset;
    eeprom_m95_read(M95_1, addr, buf, size);
    return size;
}

static int write1(long offset, const uint8_t *buf, size_t size)
{
    uint32_t addr = eeprom_m95_1.addr + offset;
    eeprom_m95_write(M95_1, addr, (uint8_t *)buf, size);
    return size;
}

static int read2(long offset, uint8_t *buf, size_t size)
{
    /* You can add your code under here. */
    uint32_t addr = eeprom_m95_2.addr + offset;
    eeprom_m95_read(M95_2, addr, buf, size);
    return size;
}

static int write2(long offset, const uint8_t *buf, size_t size)
{
    uint32_t addr = eeprom_m95_2.addr + offset;
    eeprom_m95_write(M95_2, addr, (uint8_t *)buf, size);
    return size;
}
