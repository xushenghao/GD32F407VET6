#ifndef __STEP_MOTOR_H
#define __STEP_MOTOR_H
#include "lib.h"
#include "gpios.h"

typedef enum
{
    STEP_MOTOR_1 = 0,
    STEP_MOTOR_MAX
} step_motor_index_e; // 步进电机数组索引

typedef struct
{
    gpio_t *en;             // 使能
    gpio_t *dir;            // 方向
    TIM_HandleTypeDef *tim; // 定时器
    uint8_t chan;           // 定时器通道
} step_motor_hardware_t;    // 步进电机硬件

typedef struct
{
    step_motor_hardware_t hardware; // 硬件
    uint16_t default_arr;           // 默认自动重装值
    uint8_t status;                 // 状态
    uint32_t pluse_count;           // 脉冲计数
    int32_t goal_position;          // 目标位置
    int32_t last_position;          // 上一次位置
    int32_t cur_position;           // 当前位置
    int32_t pos_bias;               // 位置偏差
    int32_t speed;                  // 速度
} step_motor_t;

/**
 * @brief   步进电机初始化
 * @param {step_motor_index_e} index    步进电机索引
 * @param {step_motor_hardware_t} hardware  硬件
 * @param {uint16_t} arr    自动重装值
 * @param {uint16_t} psc    时钟预分频数
 * @return {*}
 * @note
 */
void step_motor_init(step_motor_index_e index, step_motor_hardware_t hardware);

/**
 * @brief   步进电机启动
 * @param {step_motor_index_e} index    步进电机索引
 * @return {*}
 * @note
 */
void step_motor_start(step_motor_index_e index);

/**
 * @brief   步进电机停止
 * @param {step_motor_index_e} index    步进电机索引
 * @return {*}
 * @note
 */
void step_motor_stop(step_motor_index_e index);

// /**
//  * @brief   步进电机停止
//  * @param {step_motor_index_e} index    步进电机索引
//  * @param {uint8_t} arr    自动重装值
//  * @return {*}
//  * @note
//  */
// void step_motor_prescaler_start(step_motor_index_e index, uint16_t arr);
#endif // __STEP_MOTOR_H
