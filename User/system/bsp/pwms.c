#include "pwms.h"

// 计算频率
static void calculate_pwm_freq(pdctrl_t *pdctrl, uint32_t arr)
{
    pdctrl->sysclk = SystemCoreClock / 1000;
    pdctrl->psc = pdctrl->tim->Instance->PSC;
    pdctrl->freq = (float32)pdctrl->sysclk / (float32)(pdctrl->psc + 1) / (float32)arr;
}

/**
 * @brief 变频计算占空比
 * @param {uint32_t} arr 自动加载值
 * @return {*}
 * @note
 */
static uint16_t calculate_pwm_duty(pdctrl_t *pdctrl, uint32_t arr)
{
    float32 wid = pdctrl->pwm_wid;
    calculate_pwm_freq(pdctrl, arr);
    if (arr < pdctrl->default_arr)
    {
        wid = (pdctrl->default_arr - arr) * 10 / pdctrl->default_arr + wid;
    }

    pdctrl->arr_us = 1000.0f / pdctrl->freq;
    pdctrl->duty_percent = ((float32)wid * 100.0f) / pdctrl->arr_us;
    uint16_t duty = arr * pdctrl->duty_percent / 100;
    return duty;
}

/**
 * @brief 动态改变PWM频率
 * @param {pdctrl_t} pdctrl
 * @param {uint32_t} arr
 * @return {*}
 * @note
 */
void pwms_dynamic_frequency(pdctrl_t *pdctrl, uint32_t arr)
{
    uint16_t current_period = __HAL_TIM_GET_AUTORELOAD(pdctrl->tim);
    if (current_period == arr)
    {
        return;
    }
    PWM_STOP(pdctrl->tim, pdctrl->chan);
    __HAL_TIM_SET_AUTORELOAD(pdctrl->tim, arr);
    uint16_t duty = calculate_pwm_duty(pdctrl, arr);
    PWM_SET_DUTY(pdctrl->tim, pdctrl->chan, duty);
    PWM_START(pdctrl->tim, pdctrl->chan);
}

void pwms_set_duty(pdctrl_t *pdctrl, float32 percent)
{
    uint16_t arr = __HAL_TIM_GET_AUTORELOAD(pdctrl->tim);
    uint16_t duty = arr * percent / 100;
    PWM_SET_DUTY(pdctrl->tim, pdctrl->chan, duty);
}
