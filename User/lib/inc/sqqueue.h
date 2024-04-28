/**
 * @file sqqueue.h
 * @author xxx
 * @date 2023-06-25 13:07:02
 * @brief 提供循环队列功能
 * @copyright Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#ifndef __SQQUEUE_H
#define __SQQUEUE_H
#include <stdbool.h>
#include "data_type_def.h"

typedef struct _sqqueue_t
{
    uint8_t *base;      // 队列存储元素的首地址
    uint8_t entry_size; // 队列元素的宽度
    uint16_t sqq_len;   // 队列总长
    uint16_t front;     // 队列头下标
    uint16_t rear;      // 队列尾下标
} sqqueue_t;

/**
 * 通用循环队列伪类
 * 该队列有九个操作，分别为单元素入队列、多元素入队列、出队列，
 * 单元素撤销入队列(队尾删除)、取队列长度、判空、清空队列、遍历和删除指定位置
 */
typedef struct _sqqueue_ctrl_t
{
    sqqueue_t sqq;
    bool (*enter)(struct _sqqueue_ctrl_t *const p_this, const void *const e);                           // 单元素入队列
    bool (*string_enter)(struct _sqqueue_ctrl_t *const p_this, const void *const string, uint16_t len); // 多元素入队列
    void *(*del)(struct _sqqueue_ctrl_t *const p_this);                                                 // 出队列
    void *(*revoke)(struct _sqqueue_ctrl_t *const p_this);                                              // 撤销入队列
    uint16_t (*get_len)(const struct _sqqueue_ctrl_t *const p_this);                                    // 取队列长度
    bool (*full)(const struct _sqqueue_ctrl_t *const p_this);                                           // 判满
    void (*clear_sqq)(struct _sqqueue_ctrl_t *const p_this);                                            // 清空队列
    void (*traverse)(struct _sqqueue_ctrl_t *const p_this, void (*vi)(const void *e));                  // 遍历
    void (*remove)(struct _sqqueue_ctrl_t *const p_this, uint16_t location);                            // 删除指定位置
} sqqueue_ctrl_t;

bool sqqueue_ctrl_init(sqqueue_ctrl_t *const p_this,
                       uint8_t entry_size,
                       uint16_t sqq_len); ///< 初始化

#endif
/**
 * @}
 */
