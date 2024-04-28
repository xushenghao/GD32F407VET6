/***
 * @Author:
 * @Date: 2023-03-29 13:16:28
 * @LastEditors: xxx
 * @LastEditTime: 2023-03-30 00:34:11
 * @Description:数据类型定义
 * @email:
 * @Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#ifndef __DATA_TYPE_DEF_H_
#define __DATA_TYPE_DEF_H_
#include <stdint.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef OK
typedef enum
{
    OK = 0,
    FAIL = !OK,
} state_e;
#endif

#ifndef __IO
#define __IO volatile
#endif

typedef unsigned char BOOL;   /* boolean data */
typedef unsigned char bool_t; /* boolean data */

#if !defined(__stdint_h) && !defined(_GCC_WRAP_STDINT_H)
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned long int uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_t;
typedef signed short int int16_t;
typedef signed long int int32_t;
typedef long long int64_t;
#endif

typedef float float32;
typedef double float64;

#ifndef float32_t
typedef float float32_t;
#endif

#ifndef float64_t
typedef double float64_t;
#endif

#pragma pack(1)
typedef struct
{
    uint8_t bs[3];
} uint24_t;
typedef struct
{
    uint8_t bs[5];
} uint40_t;

typedef union
{
    float32 f;
    int32_t c;
} float32_u;

#pragma pack()

typedef uint16_t nwk_id_t;

/**
 * STANDARD BITS
 */
#ifndef BIT0
#define BIT0 (0x01u)
#define BIT1 (0x02u)
#define BIT2 (0x04u)
#define BIT3 (0x08u)
#define BIT4 (0x10u)
#define BIT5 (0x20u)
#define BIT6 (0x40u)
#define BIT7 (0x80u)
#define BIT8 (0x0100u)
#define BIT9 (0x0200u)
#define BIT10 (0x0400u)
#define BIT11 (0x0800u)
#define BIT12 (0x1000u)
#define BIT13 (0x2000u)
#define BIT14 (0x4000u)
#define BIT15 (0x8000u)
#define BIT16 (0x00010000u)
#define BIT17 (0x00020000u)
#define BIT18 (0x00040000u)
#define BIT19 (0x00080000u)
#define BIT20 (0x00100000u)
#define BIT21 (0x00200000u)
#define BIT22 (0x00400000u)

#define BIT_SET(x, b) x |= b            // 置位
#define BIT_CLR(x, b) x &= ~b           // 清零
#define BIT_IS_SET(x, b) ((x) & (b))    // 判断某一位是否为1
#define BIT_IS_CLR(x, b) (!((x) & (b))) // 判断某一位是否为0

#endif

#ifndef BF
/**
 * @brief 从一个字节中提取指定位的值
 * @return {*}
 * @note
 *> uint8_t num = 0x12;  二进制表示为00010010                <p>
 *> uint8_t bit = 2;     提取第2位（从0开始计数）             <p>
 *> uint8_t width = 1;   提取1位                            <p>
 *> uint8_t result = BF(num, bit, width);  结果为1          <p>
 */
#define BF(x, b, s) (((x) & (b)) >> (s))
#endif

#ifndef MIN
/**
 * @brief
 * @return {*}
 * @note
 *> int num1 = 10;                                  <p>
 *> int num2 = 20;                                  <p>
 *> int result = MIN(num1, num2); // 结果为10        <p>
 */
#define MIN(n, m) (((n) < (m)) ? (n) : (m))
#endif

#ifndef MAX
/**
 * @brief
 * @return {*}
 * @note
 *> int num1 = 10;                                  <p>
 *> int num2 = 20;                                  <p>
 *> int result = MAX(num1, num2); // 结果为20        <p>
 */
#define MAX(n, m) (((n) < (m)) ? (m) : (n))
#endif

#ifndef ABS
/**
 * @brief
 * @return {*}
 * @note
 *> int num = -10;
 *> int result = ABS(num); // 结果为10
 */
#define ABS(n) (((n) < 0) ? -(n) : (n))
#endif

#ifndef RANGE
#define RANGE(x, a, b) (MIN(MAX(x, a), b))
#endif

#define ARRAY_LEN(arr) (sizeof(arr)) / (sizeof(arr[0]))

#define HI_UINT16(a) (((uint16_t)(a) >> 8) & 0xFF)
#define LO_UINT16(a) ((uint16_t)(a) & 0xFF)

#define HI_1_UINT32(a) (((uint32_t)(a) >> 24) & 0xFF)
#define HI_2_UINT32(a) (((uint32_t)(a) >> 16) & 0xFF)
#define HI_3_UINT32(a) (((uint32_t)(a) >> 8) & 0xFF)
#define HI_4_UINT32(a) ((uint32_t)(a) & 0xFF)

#define LO_1_UINT8(a) (uint8_t)((a) & 0xFF)
#define LO_2_UINT8(a) (uint8_t)(((a) & 0xFF00) >> 8)
#define LO_3_UINT8(a) (uint8_t)(((a) & 0xFF0000) >> 16)
#define LO_4_UINT8(a) (uint8_t)(((a) & 0xFF000000) >> 24)

// uint32小端转大端
#define S2B_UINT32(a) \
    (((uint32_t)(a) & 0xFF000000) >> 24) + (((uint32_t)(a) & 0x00FF0000) >> 8) + (((uint32_t)(a) & 0x0000FF00) << 8) + (((uint32_t)(a) & 0x000000FF) << 24)

// uint32大端转小端
#define B2S_UINT32(a) S2B_UINT32(a)

// uint16小端转大端
#define S2B_UINT16(a) ((((uint16_t)(a) & 0xFF00) >> 8) + (((uint16_t)(a) & 0x00FF) << 8))

// uint16大端转小端
#define B2S_UINT16(a) S2B_UINT16(a)

#define BUILD_UINT16(loByte, hiByte) \
    ((uint16_t)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)))

// float32小端转大端
static inline float32 S2B_FLOAT32(float fv)
{
    float32_u _f;
    _f.f = fv;
    _f.c = S2B_UINT32(_f.c);
    return _f.f;
}

// float32大端转小端
#define B2S_FLOAT32(a) S2B_FLOAT32(a)

// 反序数组
#define REVERSE_ARRAY(arr, len)          \
    do                                   \
    {                                    \
        uint8_t _tmp;                    \
        uint16_t _i;                     \
        for (_i = 0; _i < len / 2; _i++) \
        {                                \
            _tmp = arr[_i];              \
            arr[_i] = arr[len - _i - 1]; \
            arr[len - _i - 1] = _tmp;    \
        }                                \
    } while (0);

// 比较2个数组是否相等
#define IsEqual(arr1, arr2, n) ({  \
    int _equal = 1;                \
    for (int _i = 0; _i < n; _i++) \
    {                              \
        if (arr1[_i] != arr2[_i])  \
        {                          \
            _equal = 0;            \
            break;                 \
        }                          \
    }                              \
    _equal;                        \
})

// ASSIC码转换为数字
#define ASCII_TO_NUM(c) ((c) >= '0' && (c) <= '9' ? (c) - '0' : (c) - 'A' + 10)

// 数字转换为ASSIC码
#define NUM_TO_ASCII(x) ((x) < 10 ? (x) + '0' : (x)-10 + 'A')
#endif /* __DATA_TYPE_DEF_H_ */
