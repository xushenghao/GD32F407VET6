/**
 * @file filter.h
 * @author xxx
 * @date 2023-08-08 22:59:46
 * @brief
 * @copyright Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#ifndef __FILTER_H__
#define __FILTER_H__
#include "lib.h"

typedef struct
{
    float32 Last_P; // 上次估算协方差 不可以为0 ! ! ! ! !
    float32 Now_P;  // 当前估算协方差
    float32 out;    // 卡尔曼滤波器输出
    float32 Kg;     // 卡尔曼增益
    float32 Q;      // 过程噪声协方差
    float32 R;      // 观测噪声协方差

    uint8_t filter_count;
    float32 filter_limit;
    BOOL change;
} kalman_t; // 卡尔曼滤波器

typedef struct
{
    BOOL fisrt_flag;    // 第一次标志位
    float32 alpha;      // 滤波系数    0~1
    float32 last_value; // 上次滤波结果
} lpf_t;                // 一阶低通滤波器

typedef struct
{
    uint16_t size;        // 滑动窗口大小
    float32 *window;      // 滑动窗口
    volatile float32 sum; // 滑动窗口和
    volatile float32 out; // 滤波结果
    uint16_t index;       // 滑动窗口索引
} lpf_window_t;           // 滑动窗口滤波器

void kalman_init(kalman_t *cfg);
void kalman_reset(kalman_t *cfg);
float32 kalman_update(kalman_t *cfg, float32 input);

void lpf_init(lpf_t *cfg);
float32 lpf_update(lpf_t *cfg, float32 input);
void lpf_reset(lpf_t *cfg);

lpf_window_t *lpf_window_init(uint16_t size);
void lpf_window_dinit(lpf_window_t *cfg);
float32 lpf_window_update(lpf_window_t *cfg, float32 input);
void lpf_window_reset(lpf_window_t *cfg);
#endif // __FILTER_H__
