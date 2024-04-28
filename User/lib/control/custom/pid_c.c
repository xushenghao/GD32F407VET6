#include "pid_c.h"

/**
 * @brief 设置PID控制器参数
 * @param {PID_C} *self - PID控制器结构体指针
 * @param {float32} kp - 比例系数
 * @param {float32} ki - 积分系数
 * @param {float32} kd - 微分系数
 * @param {float32} out_min - 最小输出
 * @param {float32} out_max - 最大输出
 * @return {*} - 空
 */
static void _set_ctrl_prm(struct PID_C *self, float32 kp, float32 ki, float32 kd, float32 out_min, float32 out_max)
{
    self->pri.kp = kp; /*比例系数*/
    self->pri.ki = ki; /*积分系数*/
    self->pri.kd = kd; /*微分系数*/

    self->pri.deadband = 0.5;    /*死区*/
    self->pri.maximum = out_max; /*最大输出*/
    self->pri.minimum = out_min; /*最小输出*/
    self->pri.last_error = 0;    /*上一次误差*/
    self->pri.prev_error = 0;    /*上上次误差*/
}

static float32 _PID(struct PID_C *self, float32 target, float32 feedback)
{
    /**
     * 实现PID算法
     */
    return 0;
}

void pid_c_constructor(struct PID_C *self)
{
    self->set_ctrl_prm = _set_ctrl_prm;
    self->PID = _PID;
}
