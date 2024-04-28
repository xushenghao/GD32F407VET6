#include "filter.h"
#include <math.h>
#include "osel_arch.h"
// 卡尔曼滤波
#define FILTER_COUNT 10
void kalman_init(kalman_t *cfg)
{
    cfg->Last_P = 1;
    cfg->Now_P = 0;
    cfg->out = 0;
    cfg->Kg = 0;
    cfg->Q = 0;
    cfg->R = 0.05;
    cfg->filter_count = 0;
    if (cfg->filter_limit == 0)
    {
        cfg->filter_limit = 1.0f; // 限制滤波器的最大误差
    }

    cfg->change = TRUE;
}

void kalman_reset(kalman_t *cfg)
{
    cfg->Last_P = 0;
}

float32 kalman_update(kalman_t *cfg, float32 input)
{
    if (fabs(input - cfg->out) > cfg->filter_limit)
    {
        if (cfg->filter_count < FILTER_COUNT)
        {
            cfg->filter_count++;
            cfg->change = FALSE;
        }
        else
        {
            kalman_init(cfg);
        }
    }
    else
    {
        cfg->filter_count = 0;
        cfg->change = FALSE;
    }

    // 预测协方差方程：k时刻系统估算协方差 = k-1时刻的系统协方差 + 过程噪声协方差
    cfg->Now_P = cfg->Last_P + cfg->Q;
    // 卡尔曼增益方程：卡尔曼增益 = k时刻系统估算协方差 / （k时刻系统估算协方差 + 观测噪声协方差）
    cfg->Kg = cfg->Now_P / (cfg->Now_P + cfg->R);
    // 更新最优值方程：k时刻状态变量的最优值 = 状态变量的预测值 + 卡尔曼增益 * （测量值 - 状态变量的预测值）
    cfg->out = cfg->out + cfg->Kg * (input - cfg->out); // 因为这一次的预测值就是上一次的输出值
    // 更新协方差方程: 本次的系统协方差付给 cfg->LastP 威下一次运算准备。
    cfg->Last_P = (1 - cfg->Kg) * cfg->Now_P;
    return cfg->out;
}

// 一阶滞后滤波法
void lpf_init(lpf_t *cfg)
{
    cfg->fisrt_flag = TRUE;
    cfg->last_value = 0;
    if (cfg->alpha <= 0 || cfg->alpha > 1)
    {
        cfg->alpha = 0.3f;
    }
}

float32 lpf_update(lpf_t *cfg, float32 input)
{
    float32 out;

    /***************** 如果第一次进入，则给 out_last 赋值 ******************/
    if (TRUE == cfg->fisrt_flag)
    {
        cfg->fisrt_flag = FALSE;
        cfg->last_value = input;
    }

    /*************************** 一阶滤波 *********************************/
    out = cfg->alpha * input + (1 - cfg->alpha) * cfg->last_value;
    cfg->last_value = out;

    return out;
}

void lpf_reset(lpf_t *cfg)
{
    cfg->fisrt_flag = TRUE;
}

/**
 * 滑动平均窗口滤波
 */
lpf_window_t *lpf_window_init(uint16_t size)
{
    lpf_window_t *cfg = (lpf_window_t *)osel_mem_alloc(sizeof(lpf_window_t));
    DBG_ASSERT(cfg != NULL __DBG_LINE);
    osel_memset((uint8_t *)cfg, 0, sizeof(lpf_window_t));
    cfg->size = size;
    cfg->window = (float32 *)osel_mem_alloc(sizeof(float32) * size);
    DBG_ASSERT(cfg->window != NULL __DBG_LINE);
    cfg->index = 0;
    cfg->sum = 0;
    return cfg;
}

void lpf_window_dinit(lpf_window_t *cfg)
{
    if (cfg != NULL)
    {
        if (cfg->window != NULL)
        {
            osel_mem_free(cfg->window);
        }
        osel_mem_free(cfg);
    }
}

// 滑动平均窗口重置
void lpf_window_reset(lpf_window_t *cfg)
{
    cfg->index = 0;
    cfg->sum = 0;
    // osel_memset((uint8_t *)cfg->window, 0, sizeof(float32) * cfg->size);
}

float32 lpf_window_update(lpf_window_t *cfg, float32 input)
{
    cfg->sum = 0;
    // 如果窗口未满，直接添加新值到当前索引位置
    if (cfg->index < cfg->size)
    {
        cfg->window[cfg->index++] = input;
    }
    else
    {
        // 如果窗口已满，替换最旧的值
        for (uint16_t i = 0; i < cfg->size - 1; i++)
        {
            cfg->window[i] = cfg->window[i + 1];
        }
        cfg->window[cfg->size - 1] = input;
    }
    // 计算窗口中所有值的和
    for (uint16_t i = 0; i < cfg->index; i++)
    {
        cfg->sum += cfg->window[i];
    }
    // 计算平均值
    cfg->out = cfg->sum / cfg->index;
    return cfg->out;
}
