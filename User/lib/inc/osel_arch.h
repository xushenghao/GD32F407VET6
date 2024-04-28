/***
 * @Author:
 * @Date: 2023-04-04 08:13:11
 * @LastEditors: xxx
 * @LastEditTime: 2023-04-04 08:16:58
 * @Description:
 * @email:
 * @Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#ifndef __OSEL_ARCH_H__
#define __OSEL_ARCH_H__

#include "lib.h"
#define hal_int_state_t char
#ifdef STM32
#include "main.h"
#define HAL_ENTER_CRITICAL(__HANDLE__)        \
    do                                        \
    {                                         \
        if ((__HANDLE__)->Lock == HAL_LOCKED) \
        {                                     \
            return HAL_BUSY;                  \
        }                                     \
        else                                  \
        {                                     \
            (__HANDLE__)->Lock = HAL_LOCKED;  \
        }                                     \
    } while (0U)

#define HAL_EXIT_CRITICAL(__HANDLE__)      \
    do                                     \
    {                                      \
        (__HANDLE__)->Lock = HAL_UNLOCKED; \
    } while (0U)
#else
#define HAL_ENTER_CRITICAL(__HANDLE__)

#define HAL_EXIT_CRITICAL(__HANDLE__)

#endif

#define osel_memset _memset
#define osel_memcmp _memcmp
#define osel_memcpy _memcpyL
#define osel_memcpyr _memcpyR
#define osel_reverse _reverse
#define osel_mem_alloc _malloc
#define osel_mem_free _free
#define osel_mem_alloc2 _malloc2
#define osel_mem_free2 _free2
#define osel_mstrlen _mstrlen

static inline void *_malloc(uint32_t size)
{
    return mymalloc(SRAMIN, size);
}

static inline void _free(void *ptr)
{
    myfree(SRAMIN, ptr);
}

static inline void *_malloc2(uint32_t size)
{
    return mymalloc(SRAMEX, size);
}

static inline void _free2(void *ptr)
{
    myfree(SRAMEX, ptr);
}

/**
 * @brief Fills a block of memory with a given value.
 *
 * @param dst The destination block of memory.
 * @param value The value to fill the memory with.
 * @param size The size of the memory block, in bytes.
 */
static inline void _memset(uint8_t *dst, uint8_t value, uint16_t size)
{
    while (size--)
    {
        *dst++ = value;
    }
}

/**
 * @brief Compares two blocks of memory for equality.
 *
 * @param[in] dst The first block of memory to compare.
 * @param[in] src The second block of memory to compare.
 * @param[in] size The number of bytes to compare.
 *
 * @return 0 if the blocks of memory are equal, -1 otherwise.
 */
static inline int8_t _memcmp(const uint8_t *dst, const uint8_t *src, uint16_t size)
{
    while (size--)
    {
        if (*dst++ != *src++)
        {
            return -1;
        }
    }
    return 0;
}

/**
 * @brief Copies data from a source buffer to a destination buffer in a forward direction.
 *
 * @param dst The destination buffer.
 * @param src The source buffer.
 * @param size The number of bytes to copy.
 */
static inline void _memcpyL(uint8_t *dst, const uint8_t *src, uint16_t size)
{
    while (size--)
    {
        *dst++ = *src++;
    }
}

/**
 * @brief Copies data from a source buffer to a destination buffer in reverse order.
 *
 * @param dst The destination buffer.
 * @param src The source buffer.
 * @param size The number of bytes to copy.
 */
static inline void _memcpyR(uint8_t *dst, const uint8_t *src, uint16_t size)
{
    // dst is a pointer to the last byte of the destination buffer
    // src is a pointer to the first byte of the source buffer
    // size is the number of bytes to copy

    // decrement the destination pointer by the size, since we want to write to the last byte of the buffer
    dst = dst + (size - 1);

    // loop through each byte in the buffer, copying from the source to the destination in reverse order
    while (size--)
    {
        // write the next byte from the source to the destination
        *dst-- = *src++;
    }
}

/**
 * @brief Reverses the order of bytes in a buffer
 *
 * @param buf The buffer to reverse
 * @param len The length of the buffer, in bytes
 */
static inline void _reverse(uint8_t *buf, uint16_t len)
{
    uint8_t temp = 0;
    uint16_t i;
    for (i = 0; i < len / 2; i++)
    {
        temp = buf[i];
        buf[i] = buf[len - i - 1];
        buf[len - i - 1] = temp;
    }
}
/**
 * @brief Returns the length of a null-terminated string
 *
 * @param s The string to measure
 * @return The length of the string, not including the null terminator
 */
static inline unsigned int _mstrlen(const unsigned char *s)
{
    const unsigned char *ss = s;
    while (*ss)
        ss++;

    return ss - s;
}

#endif // __OSEL_ARCH_H__
