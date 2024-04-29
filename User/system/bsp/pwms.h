/**
 * @file pwms.h
 * @brief Header file for PWMs module.
 *
 * This file contains the declarations and documentation for the PWMs module.
 *
 * @author xxx
 * @date 2023-12-27 14:44:03
 * @version 1.0
 * @copyright Copyright (c) 2024 by xxx, All Rights Reserved.
 */
#ifndef __PWMS_H__
#define __PWMS_H__
#include "lib.h"
#include "stm32f4xx_hal_tim.h"

/**
 * @brief  Starts the PWM for a specific channel
 * @param  TIMx: TIM instance
 * @param  CHx: Channel to be started
 * @retval None
 */
#define PWM_START(TIMx, CHx)          \
    do                                \
    {                                 \
        HAL_TIM_PWM_Start(TIMx, CHx); \
    } while (__LINE__ == -1)

/**
 * @brief  Stops the PWM for a specific channel
 * @param  TIMx: TIM instance
 * @param  CHx: Channel to be stopped
 * @retval None
 */
#define PWM_STOP(TIMx, CHx)          \
    do                               \
    {                                \
        HAL_TIM_PWM_Stop(TIMx, CHx); \
    } while (__LINE__ == -1)

/**
 * @brief  Sets the duty cycle for a specific channel
 * @param  TIMx: TIM instance
 * @param  CHx: Channel to be set
 * @param  DUTY: Duty cycle value (0-100)
 * @retval None
 */
#define PWM_SET_DUTY(TIMx, CHx, DUTY)           \
    do                                          \
    {                                           \
        __HAL_TIM_SET_COMPARE(TIMx, CHx, DUTY); \
    } while (__LINE__ == -1)

#define PWM_GET_ARR(TIMx) __HAL_TIM_GET_AUTORELOAD(TIMx)

typedef struct
{
    TIM_HandleTypeDef *tim; // 定时器
    uint8_t chan;           // 定时器通道
    uint16_t default_arr;
    uint8_t pwm_wid; // PWM正频宽

    // 内部改变
    uint32_t sysclk;
    uint32_t psc; // 预分频系数
    uint32_t arr;
    float32 freq;
    float32 arr_us;
    float32 duty_percent;
} pdctrl_t;

void pwms_dynamic_frequency(pdctrl_t *pdctrl, uint32_t arr); // 动态改变PWM频率
#endif                                                       // __PWMS_H__
