/*
 * @File: debug.c
 * @Descripttion:
 * @Version: 1.0
 * @Author:
 * @Date: 2022-12-10 20:15:01
 * @LastEditors: xxx
 * @LastEditTime: 2023-08-08 14:39:18
 */
#include "../inc/debug.h"

#ifndef STM32
BOOL DBG_ASSERT(uint8_t cond _DBG_LINE_)
{
    do
    {
        if ((cond) == FALSE)
        {
            LOG_ERR("DBG_ASSERT:%d", line);
            return FALSE;
        }
    } while (__LINE__ == -1);
    return TRUE;
}

#else
#define __no_init __attribute__((zero_init)) // 变量不初始化为0,keil下需要定义，并在options for target中设置noInit

uint16_t dbg_line;
BOOL DBG_ASSERT(uint8_t cond _DBG_LINE_)
{
    do
    {
        if ((cond) == FALSE)
        {
            dbg_line = line;
            while (1)
            {
                LOG_ERR("DBG_ASSERT:%d", line);
            }
        }
    } while (__LINE__ == -1);
    return TRUE;
}
#endif
