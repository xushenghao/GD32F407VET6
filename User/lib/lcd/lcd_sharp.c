/**
 * @file lcd_sharp.c
 * @author xxx
 * @date 2024-03-27 13:50:16
 * @brief LCD波特率设置至少2Mhz
 * @copyright Copyright (c) 2024 by xxx, All Rights Reserved.
 */
#include "lcds.h"
#include "delay.h"

#define LCD_CMD_CLEAR 0x56  // 0b01010110
#define LCD_CMD_UPDATE 0x93 // 0b10010011
#define LCD_CMD_NOP 0x00
#define CLEAR_COLOR 0xff

static uint8_t *change_line;

static uint8_t pri[COL_DOT_MAX_400 * LIN_DOT_MAX_240 / 8] __attribute__((section(".sram2"))); // 显示缓存
// pri_send 大小为 COL_DOT_MAX_400 * LIN_DOT_MAX_240 / 8 + LIN_DOT_MAX_240 * 2 + 3，原因是_flush全部数据时最大需要的字节数
static uint8_t pri_send[COL_DOT_MAX_400 * LIN_DOT_MAX_240 / 8 + LIN_DOT_MAX_240 * 2 + 3] __attribute__((section(".sram2"))); // 发送缓存
static uint8_t pri_template[COL_DOT_MAX_400 * LIN_DOT_MAX_240 / 8];                                                          // 模版

/**
 * @brief 夏普LCD片选
 * @param {sharp_t} *lcd
 * @param {uint8_t} sta
 * @return {*}
 * @note
 */
static int32_t sharp_chip_select(lcd_t *lcd, uint8_t sta)
{
    DBG_ASSERT(lcd != NULL __DBG_LINE);
    DBG_ASSERT(lcd->info.spi != NULL __DBG_LINE);
    if (sta == 1)
    {
        lcd->info.spi->gpios.cs->set(*lcd->info.spi->gpios.cs);
    }
    else
    {
        lcd->info.spi->gpios.cs->reset(*lcd->info.spi->gpios.cs);
    }
    return 0;
}

/**
 * @brief 夏普LCD单字节传输
 * @param {sharp_t} *lcd
 * @param {uint8_t} data
 * @return {*}
 * @note
 */
static BOOL sharp_transmit_byte(lcd_t *lcd, uint8_t data)
{
    uint16_t j = 0;
    BOOL ret = TRUE;
    DBG_ASSERT(lcd != NULL __DBG_LINE);
    DBG_ASSERT(lcd->info.spi != NULL __DBG_LINE);
    LL_SPI_TransmitData8(lcd->info.spi->spi, data);
    while (LL_SPI_IsActiveFlag_TXE(lcd->info.spi->spi) == 0)
    {
        j++;
        if (j > 1000)
        {
            ret = FALSE;
            return ret;
        }
    }
    return ret;
}

/**
 * @brief lcd清屏
 * @param {sharp_t} *lcd
 * @return {*}
 * @note
 */
static int32_t _clear(lcd_t *lcd)
{
    DBG_ASSERT(lcd != NULL __DBG_LINE);
    DBG_ASSERT(lcd->info.spi != NULL __DBG_LINE);
    sharp_chip_select(lcd, TRUE);
    sharp_transmit_byte(lcd, LCD_CMD_CLEAR);
    sharp_transmit_byte(lcd, LCD_CMD_NOP);
    sharp_transmit_byte(lcd, LCD_CMD_NOP);
    sharp_chip_select(lcd, FALSE);

    lcd->info.clear_flag = FALSE;
    osel_memset((uint8_t *)change_line, 1, lcd->info.height);
    return 0;
}

/**
 * @brief 初始化，包括硬件初始化和软件初始化
 * @param {lcd_t} *lcd
 * @return {*}
 */
static int32_t _init(lcd_t *lcd)
{
    lcd->info.on = 0xff;
    uint16_t size = (lcd->info.width / 8) * (lcd->info.height);
    lcd->info.pri = (void *)pri;
    lcd->info.pri_send = (void *)pri_send;
    lcd->info.pri_template = (void *)pri_template;

    osel_memset((uint8_t *)lcd->info.pri, CLEAR_COLOR, size);
    osel_memset((uint8_t *)lcd->info.pri_send, CLEAR_COLOR, size);
    osel_memset((uint8_t *)lcd->info.pri_template, CLEAR_COLOR, size);

    change_line = (uint8_t *)osel_mem_alloc(lcd->info.height);
    osel_memset((uint8_t *)change_line, 0, lcd->info.height);

    lcd->driver.onoff(lcd, TRUE);
    lcd->driver.clear(lcd);
    return 0;
}

/**
 * @brief 拷贝模板
 * @param {lcd_t} *lcd
 * @param {template_copy_type_e} dir dir 0,拷贝pri_template到pri；1，拷贝pri到pri_template; 其余清空
 * @return {*}
 * @note
 */
static void _copy_templete(lcd_t *lcd, template_copy_type_e dir)
{
    uint16_t size = (lcd->info.width / 8) * (lcd->info.height);
    if (dir == TEMPLATE_COPY_FROM_PRI)
    {
        osel_memcpy((uint8_t *)lcd->info.pri_template, (uint8_t *)lcd->info.pri, size);
    }
    else if (dir == TEMPLATE_COPY_TO_PRI)
    {
        // 判断pri_template和pri不一致的地方将change_line置1
        for (uint16_t i = 0; i < size; i++)
        {
            if (((uint8_t *)lcd->info.pri_template)[i] != ((uint8_t *)lcd->info.pri)[i])
            {
                change_line[i / (lcd->info.width / 8)] = 1;
            }
        }
        osel_memcpy((uint8_t *)lcd->info.pri, (uint8_t *)lcd->info.pri_template, size);
    }
    else
    {
        osel_memset((uint8_t *)lcd->info.pri, CLEAR_COLOR, size);
        osel_memset((uint8_t *)lcd->info.pri_template, CLEAR_COLOR, size);
        osel_memset((uint8_t *)change_line, 1, lcd->info.height);
    }
}

/**
 * @brief 清除显存
 * @param {lcd_t} *lcd
 * @param {uint16_t} min_row 最小行
 * @param {uint16_t} max_row 最大行
 * @return {*}
 * @note
 */
static void _clear_ram(lcd_t *lcd, uint16_t min_row, uint16_t max_row)
{
    uint8_t(*lcd_gram)[lcd->info.width / 8] = (uint8_t(*)[lcd->info.width / 8]) lcd->info.pri;
    for (uint16_t i = min_row; i < max_row; i++)
    {
        osel_memset((uint8_t *)lcd_gram[i], CLEAR_COLOR, (lcd->info.width / 8));
        change_line[i] = 1;
    }
}

/**
 * @brief
 * @param {lcd_t} *lcd
 * @param {uint16_t} *color
 * @param {uint32_t} len
 * @return {*}
 */
static void _flush(lcd_t *lcd)
{
    lcd->info.flush_use_time = sys_millis();
    uint16_t tmp = 0;
    uint32_t length = 0;
    DBG_ASSERT(lcd != NULL __DBG_LINE);
    DBG_ASSERT(lcd->info.spi != NULL __DBG_LINE);
    uint16_t l = lcd->info.height;
    uint16_t r = lcd->info.width / 8;
    uint8_t(*lcd_gram)[lcd->info.width / 8] = (uint8_t(*)[lcd->info.width / 8]) lcd->info.pri;
    uint8_t *lcd_gram_send = (uint8_t *)lcd->info.pri_send;
    sharp_chip_select(lcd, TRUE);

    *lcd_gram_send++ = LCD_CMD_UPDATE;
    for (uint16_t line = 0; line < l; line++)
    {
        tmp = line + 1; // 行移动

        if (change_line != NULL && change_line[line] == 1)
        {
            *lcd_gram_send++ = (uint8_t)tmp;
            osel_memcpy(lcd_gram_send, lcd_gram[line], r);
            lcd_gram_send += r;
            *lcd_gram_send++ = LCD_CMD_NOP;
            change_line[line] = 0;
        }
    }
    *lcd_gram_send++ = LCD_CMD_NOP;
    *lcd_gram_send++ = LCD_CMD_NOP;
    length = (uint32_t)lcd_gram_send - (uint32_t)lcd->info.pri_send;
    if (length > 4)
    {
        lcd->info.spi->interface.spi_dma_send(lcd->info.spi, (uint8_t *)lcd->info.pri_send, (uint32_t)lcd_gram_send - (uint32_t)lcd->info.pri_send);
    }

    sharp_chip_select(lcd, FALSE);
    lcd->info.flush_use_time = sys_millis() - lcd->info.flush_use_time;
}

static void _flush_clear(lcd_t *lcd)
{
    lcd->driver.flush(lcd);
    uint16_t size = (lcd->info.width / 8) * (lcd->info.height);
    osel_memset((uint8_t *)lcd->info.pri, CLEAR_COLOR, size);
    osel_memset((uint8_t *)change_line, 1, lcd->info.height);
}


static int32_t _get_point(lcd_t *lcd, uint16_t x, uint16_t y)
{
    // 优化后的读点函数
    // 直接计算点的索引和位位置
    uint16_t index = y;
    uint8_t bit_pos = x % 8;
    uint8_t mask = 0x01 << bit_pos;
    uint8_t(*lcd_gram)[lcd->info.width / 8] = (uint8_t(*)[lcd->info.width / 8]) lcd->info.pri;

    // 直接操作位而不使用中间变量
    if (lcd_gram[index][x / 8] & mask)
    {
        return (uint8_t)WHITE; // 该点为白色
    }
    else
    {
        return (uint8_t)BLACK; // 该点为黑色
    }
}

static int32_t _set_point(lcd_t *lcd, uint16_t x, uint16_t y, uint16_t color)
{
    // 优化后的画点函数
    // 直接计算点的索引和位位置
    uint16_t index = y;
    uint8_t bit_pos = x % 8;
    uint8_t mask = 0x01 << bit_pos;
    uint8_t(*lcd_gram)[lcd->info.width / 8] = (uint8_t(*)[lcd->info.width / 8]) lcd->info.pri;

    // 直接操作位而不使用中间变量
    if (color == (uint8_t)WHITE)
    {
        lcd_gram[index][x / 8] |= mask;
    }
    else
    {
        lcd_gram[index][x / 8] &= ~mask;
    }

    // 如果change_line数组可用，则将行标记为已更改
    if (change_line != NULL)
    {
        change_line[index] = 1;
    }

    return 0;
}

/**
 * @brief 将一块区域设定为某种颜色
 * @param {lcd_t} *lcd
 * @param {uint16_t} sx
 * @param {uint16_t} ex
 * @param {uint16_t} sy
 * @param {uint16_t} ey
 * @param {uint16_t} color
 * @return {*}
 */
static int32_t _color_fill(lcd_t *lcd, uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey, uint16_t color)
{
    return 0;
}

/**
 * @brief 显示或关闭
 * @param {lcd_t} *lcd
 * @param {uint8_t} sta
 * @return {*}
 */
static int32_t _onoff(lcd_t *lcd, uint8_t sta)
{
    if (lcd->info.on != sta)
    {
        lcd->info.on = sta;
        if (sta == 1)
        {
            lcd->info.disp->set(*lcd->info.disp);
        }
        else
        {
            lcd->info.disp->reset(*lcd->info.disp);
        }
    }
    return 0;
}

/**
 * @brief 设置显存扫描方向
 * @param {lcd_t} *lcd
 * @param {uint8_t} scan_dir 0 = 0度扫描，1 = 180度扫描
 * @return {*}
 */
static void _set_dir(lcd_t *lcd, uint8_t scan_dir)
{
    if (lcd->info.scandir != scan_dir)
    {
        lcd->info.scandir = scan_dir;
        lcd->info.need_build = TRUE;
        if (scan_dir == 0)
        {
        }
        else
        {
        }
        uint16_t size = (lcd->info.width / 8) * (lcd->info.height);
        osel_memset((uint8_t *)lcd->info.pri, CLEAR_COLOR, size);
        lcd->driver.clear(lcd);
    }
}

static uint8_t _get_dir(lcd_t *lcd)
{
    return lcd->info.scandir;
}

/**
 * @brief
 * @param {lcd_t} *lcd
 * @param {uint16_t} color
 * @return {*}
 * @note
 */
static void _full_fill(lcd_t *lcd, uint16_t color)
{
    uint16_t size = (lcd->info.width / 8) * (lcd->info.height);
    osel_memset((uint8_t *)lcd->info.pri, (uint8_t)color, size);
    osel_memset((uint8_t *)change_line, 1, lcd->info.height);
}

/**
 * @brief 填充矩形区域,这个函数是用来填充数据的
 * @param {lcd_t} *lcd
 * @param {uint16_t} sx
 * @param {uint16_t} ex
 * @param {uint16_t} sy
 * @param {uint16_t} ey
 * @param {uint16_t} *color
 * @return {*}
 */
static int32_t _fill(lcd_t *lcd, uint16_t x, uint16_t y, uint8_t font_width, uint8_t font_hight, uint8_t *buf)
{

    return 0;
}

/**
 * @brief
 * @param {lcd_t} *lcd
 * @param {uint16_t} sx
 * @param {uint16_t} ex
 * @param {uint16_t} sy
 * @param {uint16_t} ey
 * @return {*}
 */
static int32_t _prepare_display(lcd_t *lcd, uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey)
{
    return 0;
}

/**
 * @brief
 * @param {lcd_t} *lcd
 * @param {uint8_t} sta
 * @return {*}
 */
static void _backlight(lcd_t *lcd, uint8_t sta) {}

/**
 * @brief 设置清屏标志
 * @param {lcd_t} *lcd
 * @return {*}
 * @note
 */
static void _set_clear_flag(lcd_t *lcd)
{
    lcd->info.clear_flag = TRUE;
    uint16_t size = (lcd->info.width / 8) * (lcd->info.height);
    osel_memset((uint8_t *)lcd->info.pri, CLEAR_COLOR, size);
    osel_memset((uint8_t *)change_line, 0, lcd->info.height);
}

/**
 * @brief  获取清屏标志
 * @param {lcd_t} *lcd
 * @return {*}
 * @note
 */
static BOOL _get_clear_flag(lcd_t *lcd)
{
    return lcd->info.clear_flag;
}

void lcd_sharp_init(lcd_driver_t *driver)
{
    DBG_ASSERT(driver != NULL __DBG_LINE);
    driver->init = _init;                     // 已实现
    driver->set_point = _set_point;           // 已实现
    driver->get_point = _get_point;           // 已实现
    driver->color_fill = _color_fill;         // 已实现
    driver->onoff = _onoff;                   // 已实现
    driver->set_dir = _set_dir;               // 已实现
    driver->get_dir = _get_dir;               // 已实现
    driver->flush = _flush;                   // 已实现
    driver->flush_clear = _flush_clear;       // 已实现
    driver->clear = _clear;                   // 已实现
    driver->set_clear_flag = _set_clear_flag; // 已实现
    driver->get_clear_flag = _get_clear_flag; // 已实现
    driver->full_fill = _full_fill;           // 已实现
    driver->copy_templete = _copy_templete;   // 已实现
    driver->clear_ram = _clear_ram;           // 已实现

    // 以下不实现
    driver->fill = _fill;
    driver->prepare_display = _prepare_display;
    driver->backlight = _backlight;
}
