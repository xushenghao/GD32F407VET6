/*
 * @Author:
 * @day: 2023-04-11 08:21:19
 * @LastEditors: xxx
 * @LastEditTime: 2023-08-15 10:14:58
 * @Description:
 * email:
 * Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#include "../inc/lib.h"
#include <stdio.h>
#include <string.h>

const uint8_t _days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const uint16_t _month_days[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
static uint32_t crc32_table[256]; // CRC32表

uint32_t cpu_encrypt(void)
{
    uint32_t cpuid[3];
    // 获取CPU唯一的ID
    cpuid[0] = *(uint32_t *)(UID_BASE);
    cpuid[1] = *(uint32_t *)(UID_BASE + 4);
    cpuid[2] = *(uint32_t *)(UID_BASE + 8);
    // 加密算法,很简单的加密算法
    uint32_t encrypt_code = (cpuid[0] >> 3) + (cpuid[1] >> 1) + (cpuid[2] >> 2);
    return encrypt_code;
}

// 判断加密
BOOL cpu_judge_encrypt(uint32_t cupid_encrypt)
{
    uint32_t cpuid[4];
    // 获取CPU唯一的ID
    cpuid[0] = *(uint32_t *)(UID_BASE);
    cpuid[1] = *(uint32_t *)(UID_BASE + 4);
    cpuid[2] = *(uint32_t *)(UID_BASE + 8);
    // 加密算法,很简单的加密算法
    cpuid[3] = (cpuid[0] >> 3) + (cpuid[1] >> 1) + (cpuid[2] >> 2);
    // 检查Flash中的UID是否合法
    return (cupid_encrypt == cpuid[3]);
}

/**
 * @brief Generate the CRC32 lookup table.
 *
 * This function generates the CRC32 lookup table used for fast computation of the
 * CRC32 checksum. The table is generated using the polynomial 0xEDB88320, which is a
 * common polynomial used in CRC32 calculations.
 */
static void generate_crc32_table()
{
    static BOOL is_init = FALSE;
    if (is_init)
    {
        return;
    }
    uint32_t polynomial = 0xEDB88320;
    for (uint32_t i = 0; i < 256; i++)
    {
        uint32_t crc = i;
        for (uint32_t j = 0; j < 8; j++)
        {
            crc = (crc & 1) ? (crc >> 1) ^ polynomial : crc >> 1;
        }
        crc32_table[i] = crc;
    }
    is_init = TRUE;
}

/**
 * @brief 版本号1.0拆解成1和0
 * @param {uint8_t} *version_str
 * @param {uint8_t} *hi
 * @param {uint8_t} *lo
 * @return {*}
 */
void version_split(uint8_t *version_str, uint8_t *hi, uint8_t *lo)
{
    uint8_t flag = 1;

    for (uint8_t i = 0; version_str[i] != '\0'; i++)
    {
        if (version_str[i] == '.')
        {
            flag = 0;
            continue;
        }

        if (flag)
        {
            *hi = *hi * 10 + (version_str[i] - '0');
        }
        else
        {
            *lo = *lo * 10 + (version_str[i] - '0');
        }
    }
}

// 反序数组
void reverse(uint8_t *buf, uint16_t len)
{
    uint8_t tmp;
    uint16_t i;
    for (i = 0; i < len / 2; i++)
    {
        tmp = buf[i];
        buf[i] = buf[len - i - 1];
        buf[len - i - 1] = tmp;
    }
}

/***
 * @brief 判断是否在数组中
 * @param {uint8_t} *arr 数组
 * @param {uint8_t} len     数组长度
 * @param {uint8_t} val    要判断的值
 * @return {*} TRUE: 在数组中
 */
BOOL is_in_array(uint16_t *arr, uint16_t len, uint16_t val)
{
    uint16_t i;
    for (i = 0; i < len; i++)
    {
        if (arr[i] == val)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/**
 * 计算并返回指定数据区域crc的值
 *
 * @param data:  待计算的数据区首地址
 * @param length:  待计算的数据区长度
 *
 * @return crc计算的结果
 */
uint16_t crc16_compute(const uint8_t *const data, uint16_t length)
{
    uint16_t crcVal = 0xffff;
    const uint8_t *ptr = data;

    for (uint16_t i = 0; i < length; i++)
    {
        crcVal ^= (uint16_t)*ptr++;

        for (uint8_t j = 0; j < 8; j++)
        {
            if (crcVal & 0x0001)
            {
                crcVal = (crcVal >> 1) ^ 0x8401;
            }
            else
            {
                crcVal >>= 1;
            }
        }
    }

    return crcVal;
}

/**
 * @brief Calculate the CRC32 value of a data buffer.
 *
 * This function calculates the CRC32 value of a data buffer using the lookup table method.
 * The lookup table is generated using the polynomial 0xEDB88320, which is a common polynomial used in CRC32 calculations.
 *
 * @param data The data buffer to calculate the CRC32 value of.
 * @param length The length of the data buffer in bytes.
 * @return The CRC32 value of the data buffer.
 */
uint32_t crc32_compute(const uint8_t *const data, uint16_t length)
{
    generate_crc32_table();
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++)
    {
        crc = (crc >> 8) ^ crc32_table[(crc ^ data[i]) & 0xFF];
    }
    return crc ^ 0xFFFFFFFF;
}

/**
 * 计算并返回指定数据区域异或的值
 *
 * @param data:  待计算的数据区首地址
 * @param length:  待计算的数据区长度
 *
 * @return 异或计算的结果
 */
uint8_t xor_compute(const uint8_t *const data, uint16_t length)
{
    uint16_t i;
    const uint8_t *ptr = data;
    uint8_t xor = 0;
    for (i = 0; i < length; i++)
    {
        xor ^= *ptr;
        ptr++;
    }
    return xor;
}

// 通过bit位获取置1个数量
uint8_t get_bit_num(uint8_t bit)
{
    uint8_t num = 0;
    while (bit)
    {
        if (bit & 0x01)
        {
            num++;
        }
        bit >>= 1;
    }
    return num;
}

// 通过bit位获取置1的位置
BOOL is_bit_set(int x, int k)
{
    int mask = 1 << k;
    return (x & mask) != 0;
}

// 判断数组是否全是同一个值
BOOL is_same_value(uint8_t *buf, uint16_t len, uint8_t value)
{
    uint16_t i;
    for (i = 0; i < len; i++)
    {
        if (buf[i] != value)
        {
            return FALSE;
        }
    }
    return TRUE;
}

// 检查是否是闰年
uint8_t isLeap(uint16_t year)
{
    return (year % 400 == 0) || (year % 100 != 0 && year % 4 == 0);
}

// 计算一年中的第几天
uint16_t dayOfyear(uint16_t year, uint8_t month, uint8_t day)
{
    uint8_t month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint16_t total;
    total = day;
    if (month > 2 && isLeap(year))
        total += 1;
    for (uint8_t i = 0; i < month - 1; i++)
    {
        total += month_days[i];
    }
    return total;
}

// 计算一年中的第几周
uint16_t weekOfyear(uint16_t year, uint8_t month, uint8_t day)
{
    uint16_t day_of_year = dayOfyear(year, month, day);
    return (day_of_year - 1) / 7 + 1;
}

// 获取今天星期几
uint8_t get_weekday(uint16_t year, uint8_t month, uint8_t day)
{
    uint8_t w = 0;
    if (month == 1 || month == 2)
    {
        month += 12;
        year--;
    }
    w = (day + 2 * month + 3 * (month + 1) / 5 + year + year / 4 - year / 100 + year / 400) % 7;
    return w + 1;
}

// 传入十六进制0x23 返回十进制23
uint8_t hex_format_dec(uint8_t hex)
{
    char buf[4];
    osel_memset((uint8_t *)buf, 0, 4);
    sprintf(buf, "%x", hex);
    int dec = 0;
    int weight = 1;
    int len = strlen(buf);
    for (int i = len - 1; i >= 0; i--)
    {
        if (buf[i] >= '0' && buf[i] <= '9')
        {
            dec += (buf[i] - '0') * weight;
        }
        else if (buf[i] >= 'A' && buf[i] <= 'F')
        {
            dec += (buf[i] - 'A' + 10) * weight;
        }
        else if (buf[i] >= 'a' && buf[i] <= 'f')
        {
            dec += (buf[i] - 'a' + 10) * weight;
        }
        weight *= 10;
    }
    return dec;
}

// 传入十进制23 返回十六进制0x23
uint8_t dec_format_hex(uint8_t dec)
{
    char buf[4];
    osel_memset((uint8_t *)buf, 0, 4);
    sprintf(buf, "%d", dec);
    uint8_t hex = 0;
    uint8_t len = strlen(buf);
    for (uint8_t i = 0; i < len; i++)
    {
        char c = buf[i];
        if (c >= '0' && c <= '9')
        {
            hex = hex * 16 + (c - '0');
        }
        else
        {
            continue;
        }
    }
    return hex;
}

/**
 * @brief  北京时间转时间戳
 * @param {rtc_date_t} date
 * @param {rtc_time_t} time
 * @return {*}
 * @note
 */
uint32_t time2stamp(rtc_date_t date, rtc_time_t time)
{
    uint32_t result;
    uint16_t year = date.year + 2000;
    result = (year - 1970) * 365 * 24 * 3600 + (_month_days[date.month - 1] + date.day - 1) * 24 * 3600 + (time.hour - 8) * 3600 + time.minute * 60 + time.second;
    result += (date.month > 2 && (year % 4 == 0) && (year % 100 != 0 || year % 400 == 0)) * 24 * 3600; // 闰月
    year -= 1969;
    result += (year / 4 - year / 100 + year / 400) * 24 * 3600; // 闰年
    return result;
}

/**
 * @brief  时间戳转北京时间
 * @param {uint32_t} stamp
 * @param {rtc_date_t} *date
 * @param {rtc_time_t} *time
 * @return {*}
 * @note
 */
void stamp2time(uint32_t stamp, rtc_date_t *date, rtc_time_t *time)
{
    uint32_t days;
    uint16_t leap_num;

    time->second = stamp % 60;
    stamp /= 60; // 获取分
    time->minute = stamp % 60;
    stamp += 8 * 60;
    stamp /= 60; // 获取小时
    time->hour = stamp % 24;
    days = stamp / 24;
    leap_num = (days + 365) / 1461;
    if (((days + 366) % 1461) == 0)
    {
        date->year = (days / 366) + 1970 - 2000;
        date->month = 12;
        date->day = 31;
    }
    else
    {
        days -= leap_num;
        date->year = (days / 365) + 1970 - 2000;
        days %= 365;
        days += 1;
        if (((date->year % 4) == 0) && (days == 60))
        {
            date->month = 2;
            date->day = 29;
        }
        else
        {
            if (((date->year % 4) == 0) && (days > 60))
                --days;
            for (date->month = 0; _days[date->month] < days; date->month++)
            {
                days -= _days[date->month];
            }
            ++date->month;
            date->day = days;
        }
    }
}

/**************************排序**************************/
static void swap(uint16_t *a, uint16_t *b)
{
    uint16_t t = *a;
    *a = *b;
    *b = t;
}

static int partition(uint16_t arr[], int low, int high)
{
    uint16_t pivot = arr[high];
    int i = (low - 1);
    for (int j = low; j <= high - 1; j++)
    {
        if (arr[j] < pivot)
        {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void quicksort(uint16_t arr[], int low, int high)
{
    if (low < high)
    {
        int pi = partition(arr, low, high);
        quicksort(arr, low, pi - 1);
        quicksort(arr, pi + 1, high);
    }
}
