#include "pid_g.h"
#include <math.h>

/**
 * @brief 复位PID积分及微分控制数据
 * @param {PID_G} *self
 * @return {*}
 */
static void _restctrl(struct PID_G *self)
{
    self->pri.pre_error = 0;
    self->pri.sum_iterm = 0;
}

/**
 * @brief 更新控制区间
 * @param {PID_G} *self
 * @param {float32} out_min
 * @param {float32} out_max
 * @return {*}
 * @note
 */
static void _set_range(struct PID_G *self, float32 out_min, float32 out_max)
{
    self->pri.out_max = out_max;
    self->pri.out_min = out_min;
}

/**
 * @brief 更新kp
 * @param {PID_G} *self
 * @param {float32} kp
 * @return {*}
 * @note
 */
static void _set_kp(struct PID_G *self, float32 kp)
{
    self->pri.kp = kp;
}

/**
 * @brief 更新ki
 * @param {PID_G} *self
 * @param {float32} ki
 * @return {*}
 * @note
 */
static void _set_ki(struct PID_G *self, float32 ki)
{
    self->pri.ki = ki;
}

/**
 * @brief 更新kd
 * @param {PID_G} *self
 * @param {float32} kd
 * @return {*}
 * @note
 */
static void _set_kd(struct PID_G *self, float32 kd)
{
    self->pri.kd = kd;
}

/**
 * @brief 使能积分控制
 * @param {PID_G} *self
 * @param {BOOL} enable
 * @return {*}
 * @note
 */
static void _set_ki_enable(struct PID_G *self, BOOL enable)
{
    self->pri.ki_enable = enable;
}

/**
 * @brief  使能微分控制
 * @param {PID_G} *self
 * @param {BOOL} enable
 * @return {*}
 * @note
 */
static void _set_kd_enable(struct PID_G *self, BOOL enable)
{
    self->pri.kd_enable = enable;
}

/**
 * @brief 初始化控制参数
 * @return {*}
 * @note
 */
static void _set_ctrl_prm(struct PID_G *self, float32 kp, float32 ki, float32 kd, float32 err_dead, float32 out_min, float32 out_max)
{
    g_param_t *pri = &self->pri;
    osel_memset((uint8_t *)pri, 0, sizeof(pid_g_t));
    pri->kp = kp;
    pri->ki = ki;
    pri->kd = kd;
    pri->err_dead = err_dead;
    pri->out_max = out_max;
    pri->out_min = out_min;
    pri->detach = FALSE;
}

static void _update_ctrl_prm(struct PID_G *self, float32 kp, float32 ki, float32 kd, float32 err_dead, float32 out_min, float32 out_max)
{
    g_param_t *pri = &self->pri;
    pri->kp = kp;
    pri->ki = ki;
    pri->kd = kd;
    pri->err_dead = err_dead;
    pri->out_max = out_max;
    pri->out_min = out_min;
    pri->detach = FALSE;
}

/**
 * @brief 非0时配置为积分分离+抗积分饱和PID,否则为普通抗积分饱和PID
 * @param {PID_G} *self
 * @param {float32} max_err
 * @param {BOOL} mode
 * @return {*}
 */
static void _set_cfg(struct PID_G *self, float32 max_err, BOOL mode)
{
    self->pri.err_limit = max_err;
    self->pri.detach = mode == FALSE ? FALSE : TRUE;
}

/**
 * @brief PID算法函数
 * @param {PID_G} *self
 * @param {float32} target
 * @param {float32} feedback
 * @return {*}
 * @note
 */
static float32 _PID(struct PID_G *self, float32 target, float32 feedback)
{
    float32 error = 0.0f;
    float32 insert = 0.0f;
    float32 temp_iterm = 0.0f;
    float32 temp_kd = 0.0f;

    g_param_t *pri = &self->pri;

    pri->ref = target;
    pri->feed_back = feedback;
    pri->error = pri->ref - pri->feed_back;
    error = pri->error;
    if (fabs(pri->error) <= pri->err_dead)
    {
        error = 0;
    }

    /*根据PID配置的模式,获取积分数据,进行积分累加*/
    if (pri->out >= pri->out_max)
    {
        if (fabs(error) > pri->err_limit && pri->detach)
        {
            insert = 0;
        }
        else
        {
            insert = 1;
            if (error < 0)
            {
                temp_iterm = pri->ki * error;
            }
        }
    }
    else if (pri->out <= pri->out_min)
    {
        if (fabs(error) > pri->err_limit && pri->detach)
        {
            insert = 0;
        }
        else
        {
            insert = 1;
            if (error > 0)
            {
                temp_iterm = pri->ki * error;
            }
        }
    }
    else
    {
        if (fabs(error) > pri->err_limit && pri->detach)
        {
            insert = 0;
        }
        else
        {
            insert = 1;
            temp_iterm = pri->ki * error;
        }
    }
    if (pri->ki_enable == FALSE)
    {
        insert = 0;
    }

    /* integral */
    pri->sum_iterm += temp_iterm;

    if (pri->sum_iterm > pri->out_max)
    {
        pri->sum_iterm = pri->out_max;
    }
    else if (pri->sum_iterm < pri->out_min)
    {
        pri->sum_iterm = pri->out_min;
    }

    /* differential */
    if (pri->kd_enable == TRUE)
    {
        temp_kd = pri->kd;
    }
    else
    {
        temp_kd = 0;
    }

    pri->out = pri->kp * pri->error + pri->sum_iterm * insert + (pri->error - pri->pre_error) * temp_kd;
    pri->pre_error = pri->error;
    pri->pre_feed_back = pri->feed_back;

    /*limt pid output*/
    pri->out = RANGE(pri->out, pri->out_min, pri->out_max);
    return pri->out;
}

void pid_g_constructor(struct PID_G *self)
{
    self->set_ctrl_prm = _set_ctrl_prm;
    self->update_ctrl_prm = _update_ctrl_prm;
    self->set_cfg = _set_cfg;
    self->set_kp = _set_kp;
    self->set_ki_enable = _set_ki_enable;
    self->set_ki = _set_ki;
    self->set_kd_enable = _set_kd_enable;
    self->set_kd = _set_kd;
    self->set_range = _set_range;
    self->restctrl = _restctrl;
    self->PID = _PID;
}
