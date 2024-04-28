#include "pid_hd.h"
#include <math.h>

float32 out_pos;  // 位置式pid输出
float32 Kp_watch; // 观测Kp的大小

/// <summary>
/// 杭电：设置增量式PID参数
/// </summary>
/// <param name="self"></param>
/// <param name="kp">比例参数
/// <param name="ki">积分参数
/// <param name="kd">微分参数
static void _set_ctrl_prm_position(struct PID_HD *self, float32 kp, float32 ki, float32 kd)
{
    pid_hd_position_t *pri = &self->pri_u.position;
    osel_memset((uint8_t *)pri, 0, sizeof(pid_hd_position_t));

    pri->kp = kp;
    pri->ki = ki;
    pri->kd = kd;
    pri->ki_limit = 10;  // 积分分离界限
    pri->kp_limit = 1;   // 变比例界限
    pri->err_dead = 0.5; // 控制死区范围
    pri->alpha = 0.2;    // 不完全微分系数
}

/// <summary>
/// 杭电：设置输出限幅参数
/// </summary>
/// <param name="self"></param>
/// <param name="maximum">积分输出上限
/// <param name="minimum">积分输出下限
static void _set_out_prm_position(struct PID_HD *self, float32 maximum, float32 minimum)
{
    self->pri_u.position.out_max = maximum;
    self->pri_u.position.out_min = minimum;
}

float32 out_pos_watch;
/// <summary>
/// 杭电：位置式PID控制算法
/// </summary>
/// <param name="self"></param>
/// <param name="err"></param>
/// <returns></returns>
static float32 _pid_position(struct PID_HD *self, float32 err)
{
    float32 x[3];
    self->pri_u.position.err = err;

    /*抗积分饱和*/
#if INTEGRAL_SEPARATION == 1 // 积分分离
    if (self->pri_u.position.out > self->pri_u.position.out_max)
    {
        if (self->pri_u.position.err > self->pri_u.position.ki_limit) // 积分分离
        {
            self->pri_u.position.ki_error += 0;
        }
        else
        {
            if (self->pri_u.position.err < 0) // 若偏差为负值，执行负偏差的累加
            {
                self->pri_u.position.ki_error += self->pri_u.position.err;
            }
        }
    }
    else if (self->pri_u.position.out < self->pri_u.position.out_min)
    {
        if (self->pri_u.position.err > self->pri_u.position.ki_limit) // 若偏差为负值，停止积分
        {
            self->pri_u.position.ki_error += 0;
        }
        else
        {
            if (self->pri_u.position.err > 0) // 若偏差为正值，执行正偏差的累加
            {
                self->pri_u.position.ki_error += self->pri_u.position.err;
            }
        }
    }
    else
    {
        if (fabs(err) > self->pri_u.position.ki_limit || fabs(err) < 0.4)
        {
            self->pri_u.position.ki_error += 0;
        }
        else
        {
            self->pri_u.position.ki_error += self->pri_u.position.err;
        }
    }
#else /*无积分分离操作*/
    if (fabs(err) < 0.4)
    {
        self->pri_u.position.ki_error += 0;
    }
    else
    {
        self->pri_u.position.ki_error += self->pri_u.position.err;
    }
#endif

    ///*变比例操作*/
    // if (fabs(err) > self->pri_u.position.kp_limit)
    //{
    //	self->pri_u.position.kp = self->pri_u.position.kp_big;
    // }
    // else
    //{
    //	self->pri_u.position.kp = self->pri_u.position.kp_small;
    // }

    // Kp_watch=self->pri_u.position.kp;

    /*输出*/
    if (fabs(err) < self->pri_u.position.err_dead)
    {
        self->pri_u.position.out = self->pri_u.position.pre_out;
    }
    else
    {
        x[0] = self->pri_u.position.err;
        x[1] = self->pri_u.position.ki_error;
#if INCOMPLETE_DIFFEREN == 1
        // 不完全微分项，为了解决普通PID为微分环节容易振荡的问题
        x[2] = self->pri_u.position.kd * (1 - self->pri_u.position.alpha) * (self->pri_u.position.err - self->pri_u.position.pre_error) + self->pri_u.position.alpha * self->pri_u.position.pre_error;
        out_pos = self->pri_u.position.kp * x[0] + self->pri_u.position.ki * x[1] + x[2];
#endif
        // 普通的微分环节
        x[2] = self->pri_u.position.err - self->pri_u.position.pre_error;
        out_pos = self->pri_u.position.kp * x[0] + self->pri_u.position.ki * x[1] + self->pri_u.position.kd * x[2];
        out_pos_watch = out_pos;
        self->pri_u.position.out = out_pos;
    }

    /*输出限幅*/
    if (self->pri_u.position.out > self->pri_u.position.out_max)
    {
        self->pri_u.position.out = self->pri_u.position.out_max;
    }
    if (self->pri_u.position.out < self->pri_u.position.out_min)
    {
        self->pri_u.position.out = self->pri_u.position.out_min;
    }

    // 更新误差历史
    self->pri_u.position.pre_error = self->pri_u.position.err; /*上一次误差值*/
    // 更新输出历史
    self->pri_u.position.pre_out = self->pri_u.position.out; /*上一次输出值*/

    out_pos = self->pri_u.position.out;

    return self->pri_u.position.out;
}

/// <summary>
/// 杭电：参数控制器
/// </summary>
/// <param name="self"></param>
void pid_hd_constructor(struct PID_HD *self)
{
    self->set_ctrl_prm_position = _set_ctrl_prm_position;
    self->set_out_prm_position = _set_out_prm_position;
    self->pid_position = _pid_position;
}
