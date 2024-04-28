/**
 * @file
 * @author xxx
 * @date 2023-08-29 14:28:04
 * @brief LCD驱动
 * @copyright Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#include "lcds.h"
#include "lcd_st7525.h"
#include "lcd_sharp.h"
#include <string.h>

#define BACK_COLOR WHITE
// static uint8_t dotbuf[64]; // 字符点阵缓冲

void lcd_free(lcd_t *handle)
{
    DBG_ASSERT(handle != NULL __DBG_LINE);
    osel_mem_free(handle);
}

// LCD初始化
lcd_t *lcd_create(lcd_info_t info)
{
    lcd_t *handle = (lcd_t *)osel_mem_alloc(sizeof(lcd_t));
    DBG_ASSERT(handle != NULL __DBG_LINE);
    osel_memcpy((uint8_t *)&handle->info, (uint8_t *)&info, sizeof(lcd_info_t));

    switch (info.type)
    {
    case LCD_ST7525:
        // lcd_st7525_init(&handle->driver);
        break;
    case LCD_SHARP:
        lcd_sharp_init(&handle->driver);
        break;
    default:
        lcd_free(handle);
        return NULL;
    }
    return handle;
}

void lcd_set_font_size(font_type_e font)
{
    GUI_SetBkColor(GUI_WHITE);
    GUI_SetColor(GUI_BLACK);
    switch (font)
    {
    case FONT_SMALL:
        GUI_SetFont(&GUI_FontHZ16x16);
        break;
    case FONT_MEDIUM:
        GUI_SetFont(&GUI_FontHZ24x24);
        break;
    case FONT_LARGE:
        GUI_SetFont(&GUI_FontHZ24x24);
        break;
    default:
        break;
    }
}

// 对齐显示字符串，其余部分填充背景色
int32_t lcd_put_alignment_string(lcd_t *lcd, font_type_e font, int32_t row, char *s, uint8_t alignment, BOOL reverse, uint16_t color)
{
    lcd_set_font_size(font);
    uint16_t len = GUI_GetStringDistX(s);
    switch (alignment)
    {
    case GUI_TA_LEFT:
    case GUI_TA_NORMOL:
        if (reverse == FALSE)
        {
            GUI_SetTextMode(GUI_TEXTMODE_REV); // 反转文本
            GUI_DispStringAt(s, 0, GUI_GetFontSizeY() * row);
            GUI_SetTextAlign(GUI_TA_LEFT);
            GUI_SetColor(GUI_WHITE);             // 设置文本颜色
            GUI_SetTextMode(GUI_TEXTMODE_TRANS); // 恢复文本
            GUI_SetColor(GUI_BLACK);             // 设置文本颜色
        }
        else
        {
            GUI_DispStringAt(s, 0, GUI_GetFontSizeY() * row);
            GUI_SetTextAlign(GUI_TA_LEFT);
        }
        break;
    case GUI_TA_HCENTER:
        if (reverse == TRUE)
        {
            GUI_SetTextMode(GUI_TEXTMODE_REV); // 反转文本
            GUI_DispStringAt(s, (lcd->info.width - len) / 2, GUI_GetFontSizeY() * row);
            GUI_SetColor(GUI_WHITE);             // 设置文本颜色
            GUI_SetTextMode(GUI_TEXTMODE_TRANS); // 恢复文本
            GUI_SetColor(GUI_BLACK);             // 设置文本颜色
        }
        else
        {
            GUI_DispStringAt(s, (lcd->info.width - len) / 2, GUI_GetFontSizeY() * row);
        }

        break;
    case GUI_TA_RIGHT:
        if (reverse == TRUE)
        {
            GUI_SetTextMode(GUI_TEXTMODE_REV); // 反转文本
            GUI_DispStringAt(s, 0, GUI_GetFontSizeY() * row);
            GUI_SetTextAlign(GUI_TA_RIGHT);
            GUI_SetColor(GUI_WHITE);             // 设置文本颜色
            GUI_SetTextMode(GUI_TEXTMODE_TRANS); // 恢复文本
            GUI_SetColor(GUI_BLACK);             // 设置文本颜色
        }
        else
        {
            GUI_DispStringAt(s, 0, GUI_GetFontSizeY() * row);
            GUI_SetTextAlign(GUI_TA_RIGHT);
        }

        break;
    default:
        break;
    }

    return 0;
}
