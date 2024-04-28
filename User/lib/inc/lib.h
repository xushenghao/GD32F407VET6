/***
 * @Author:
 * @Date: 2023-04-04 08:13:11
 * @LastEditors: xxx
 * @LastEditTime: 2023-04-04 10:13:21
 * @Description:
 * @email:
 * @Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#ifndef __LIB_H
#define __LIB_H
#include <stdint.h>
#include "data_type_def.h"
#include "malloc.h"
#include "data_analysis.h"
#include "osel_arch.h"
#include "debug.h"
#include "sqqueue.h"
#include "clist.h"

#define INTERNAL_EXTERN extern

#ifndef STM32
#include "log.h"
#else
#define LOG_PRINT(fmt, ...) \
    do                      \
    {                       \
    } while (0);
#define LOG_ERR(fmt, ...) \
    do                    \
    {                     \
    } while (0);
#define LOG_HEX(data, len) \
    do                     \
    {                      \
    } while (0);
#endif

typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
} rtc_date_t;

typedef struct
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} rtc_time_t;

extern uint32_t cpu_encrypt(void);                                         // CPU加密
extern BOOL cpu_judge_encrypt(uint32_t cupid_encrypt);                     // CPU判断加密
extern void version_split(uint8_t *version, uint8_t *hi, uint8_t *lo);     // 版本号1.0拆解成1和0
extern void reverse(uint8_t *buf, uint16_t len);                           // 反序数组
extern BOOL is_in_array(uint16_t *arr, uint16_t len, uint16_t val);        // 判断val是否在数组arr中
extern BOOL is_same_value(uint8_t *buf, uint16_t len, uint8_t value);      // 判断数组中的值是否都相等
extern uint16_t crc16_compute(const uint8_t *const data, uint16_t length); // CRC16校验
extern uint32_t crc32_compute(const uint8_t *const data, uint16_t length); // CRC32校验
extern uint8_t xor_compute(const uint8_t *const data, uint16_t length);    // 异或校验
extern uint8_t get_bit_num(uint8_t bit);                                   // 获取bit位的值
extern BOOL is_bit_set(int x, int k);                                      // 判断x的第k位是否为1
extern uint8_t isLeap(uint16_t year);                                      // 检查是否是闰年
extern uint16_t dayOfyear(uint16_t year, uint8_t month, uint8_t day);      // 计算一年中的第几天
extern uint16_t weekOfyear(uint16_t year, uint8_t month, uint8_t day);     // 计算一年中的第几周
extern uint8_t get_weekday(uint16_t year, uint8_t month, uint8_t day);     // 获取今天星期几
extern uint8_t hex_format_dec(uint8_t hex);                                // 十六进制转十进制
extern uint8_t dec_format_hex(uint8_t dec);                                // 十进制转十六进制
extern void quicksort(uint16_t arr[], int low, int high);                  // 快速排序

extern uint32_t time2stamp(rtc_date_t date, rtc_time_t time);               // 北京时间转时间戳
extern void stamp2time(uint32_t stamp, rtc_date_t *date, rtc_time_t *time); // 时间戳转北京时间

#endif //__LIB_H
