#include "step_motor.h"

step_motor_t step_motors[STEP_MOTOR_MAX];

/**
 * @brief   步进电机初始化
 * @param {step_motor_index_e} index    步进电机索引
 * @param {step_motor_hardware_t} hardware  硬件
 * @return {*}
 * @note
 */
void step_motor_init(step_motor_index_e index, step_motor_hardware_t hardware)
{
    DBG_ASSERT(index < STEP_MOTOR_MAX __DBG_LINE);

    step_motor_t *motor = &step_motors[index];
    step_motor_hardware_t *hd = &motor->hardware;
    hd->en = hardware.en;
    hd->dir = hardware.dir;
    hd->tim = hardware.tim;
    hd->chan = hardware.chan;
    motor->default_arr = PWM_GET_ARR(hd->tim);
    motor->hardware.en->reset(*motor->hardware.en);
    motor->hardware.dir->reset(*motor->hardware.dir);
}

/**
 * @brief   步进电机启动
 * @param {step_motor_index_e} index    步进电机索引
 * @return {*}
 * @note
 */
void step_motor_start(step_motor_index_e index)
{
    DBG_ASSERT(index < STEP_MOTOR_MAX __DBG_LINE);
    step_motor_t *motor = &step_motors[index];
    motor->hardware.en->set(*motor->hardware.en); /* 使能 */
    PWM_START(motor->hardware.tim, motor->hardware.chan);
}

/**
 * @brief   步进电机停止
 * @param {step_motor_index_e} index    步进电机索引
 * @param {uint8_t} arr    自动重装值
 * @return {*}
 * @note
 */
void step_motor_prescaler_start(step_motor_index_e index, uint16_t arr)
{
    DBG_ASSERT(index < STEP_MOTOR_MAX __DBG_LINE);
    step_motor_t *motor = &step_motors[index];
    motor->hardware.en->set(*motor->hardware.en);
    pdctrl_t pdctrl = {
        .tim = motor->hardware.tim,
        .chan = motor->hardware.chan,
        .default_arr = motor->default_arr,
        .pwm_wid = 13,
    };
    pwms_dynamic_frequency(&pdctrl, arr);
}
