#include "pid.h"
#include <math.h>

// 模糊集合
#define NL -3
#define NM -2
#define NS -1
#define ZE 0
#define PS 1
#define PM 2
#define PL 3

// 定义偏差E的范围，因为设置了非线性区间，误差在10时才开始进行PID调节，这里E的范围为10
#define MAXE (10)
#define MINE (-MAXE)
// 定义EC的范围，因为变化非常缓慢！，每次的EC都非常小，这里可以根据实际需求来调整，
#define MAXEC (10)
#define MINEC (-MAXEC)
// 定义e,ec的量化因子
#define KE 3 / MAXE
#define KEC 3 / MAXEC

// 定义输出量比例因子
#define KUP 3.0f // 这里只使用了模糊PID的比例增益
#define KUI 0.0f
#define KUD 3.0f

static const float32 fuzzyRuleKp[7][7] = {
    PL, PL, PM, PL, PS, PM, PL,
    PL, PM, PM, PM, PS, PM, PL,
    PM, PS, PS, PS, PS, PS, PM,
    PM, PS, ZE, ZE, ZE, PS, PM,
    PS, PS, PS, PS, PS, PM, PM,
    PM, PM, PM, PM, PL, PL, PL,
    PM, PL, PL, PL, PL, PL, PL};

static const float32 fuzzyRuleKi[7][7] = {
    NL, NL, NL, NL, NM, NL, NL,
    NL, NL, NM, NM, NM, NL, NL,
    NM, NM, NS, NS, NS, NM, NM,
    NM, NS, ZE, ZE, ZE, NS, NM,
    NM, NS, NS, NS, NS, NM, NM,
    NM, NM, NS, NM, NM, NL, NL,
    NM, NL, NM, NL, NL, NL, NL};

static const float32 fuzzyRuleKd[7][7] = {
    PS, PS, ZE, ZE, ZE, PL, PL,
    NS, NS, NS, NS, ZE, NS, PM,
    NL, NL, NM, NS, ZE, PS, PM,
    NL, NM, NM, NS, ZE, PS, PM,
    NL, NM, NS, NS, ZE, PS, PS,
    NM, NS, NS, NS, ZE, PS, PS,
    PS, ZE, ZE, ZE, ZE, PL, PL};

static void fuzzy(float32 e, float32 ec, FUZZY_PID_t *fuzzy_pid)
{

    float32 etemp, ectemp;
    float32 eLefttemp, ecLefttemp; // ec,e，左隶属度
    float32 eRighttemp, ecRighttemp;

    int eLeftIndex, ecLeftIndex; // 模糊位置标号
    int eRightIndex, ecRightIndex;
    e = RANGE(e, MINE, MAXE);
    ec = RANGE(ec, MINEC, MAXEC);
    e = e * KE;
    ec = ec * KEC;

    etemp = e > 3.0f ? 0.0f : (e < -3.0f ? 0.0f : (e >= 0.0f ? (e >= 2.0f ? 2.5f : (e >= 1.0f ? 1.5f : 0.5f)) : (e >= -1.0f ? -0.5f : (e >= -2.0f ? -1.5f : (e >= -3.0f ? -2.5f : 0.0f)))));
    eLeftIndex = (int)((etemp - 0.5f) + 3); //[-3,3] -> [0,6]
    eRightIndex = (int)((etemp + 0.5f) + 3);
    eLefttemp = etemp == 0.0f ? 0.0f : ((etemp + 0.5f) - e); //
    eRighttemp = etemp == 0.0f ? 0.0f : (e - (etemp - 0.5f));
    ectemp = ec > 3.0f ? 0.0f : (ec < -3.0f ? 0.0f : (ec >= 0.0f ? (ec >= 2.0f ? 2.5f : (ec >= 1.0f ? 1.5f : 0.5f)) : (ec >= -1.0f ? -0.5f : (ec >= -2.0f ? -1.5f : (ec >= -3.0f ? -2.5f : 0.0f)))));
    ecLeftIndex = (int)((ectemp - 0.5f) + 3); //[-3,3] -> [0,6]
    ecRightIndex = (int)((ectemp + 0.5f) + 3);

    ecLefttemp = ectemp == 0.0f ? 0.0f : ((ectemp + 0.5f) - ec);
    ecRighttemp = ectemp == 0.0f ? 0.0f : (ec - (ectemp - 0.5f));

    /*************************************反模糊*************************************/

    fuzzy_pid->kp = (eLefttemp * ecLefttemp * fuzzyRuleKp[eLeftIndex][ecLeftIndex] + eLefttemp * ecRighttemp * fuzzyRuleKp[eLeftIndex][ecRightIndex] + eRighttemp * ecLefttemp * fuzzyRuleKp[eRightIndex][ecLeftIndex] + eRighttemp * ecRighttemp * fuzzyRuleKp[eRightIndex][ecRightIndex]);

    fuzzy_pid->ki = (eLefttemp * ecLefttemp * fuzzyRuleKi[eLeftIndex][ecLeftIndex] + eLefttemp * ecRighttemp * fuzzyRuleKi[eLeftIndex][ecRightIndex] + eRighttemp * ecLefttemp * fuzzyRuleKi[eRightIndex][ecLeftIndex] + eRighttemp * ecRighttemp * fuzzyRuleKi[eRightIndex][ecRightIndex]);

    fuzzy_pid->kd = (eLefttemp * ecLefttemp * fuzzyRuleKd[eLeftIndex][ecLeftIndex] + eLefttemp * ecRighttemp * fuzzyRuleKd[eLeftIndex][ecRightIndex] + eRighttemp * ecLefttemp * fuzzyRuleKd[eRightIndex][ecLeftIndex] + eRighttemp * ecRighttemp * fuzzyRuleKd[eRightIndex][ecRightIndex]);
    // 对解算出的KP,KI,KD进行量化映射

    fuzzy_pid->kp = fuzzy_pid->kp * fuzzy_pid->kup;
    fuzzy_pid->ki = fuzzy_pid->ki * fuzzy_pid->kui;
    fuzzy_pid->kd = fuzzy_pid->kd * fuzzy_pid->kud;
}

/**
 * @brief SV平滑给定,步长默认为0.1，范围0-1之间，越大平滑性越差
 * @param {PID_FUZZY} *self
 * @param {float32} target_sv
 * @return {*}
 * @note
 */
static void smoothSetpoint(struct PID_FUZZY *self, float32 target_sv)
{
#if FUZZY_SUB_TYPE == PID_SUB_TYPE_POSITION
    pid_common_position_t *pri = &self->pri;
#else
    pid_common_increment_t *pri = &self->pri;
#endif
    float32 stepIn = (pri->sv_range) * 0.1f;
    float32 kFactor = 0.0f;
    if (fabs(pri->ref - target_sv) <= stepIn)
    {
        pri->ref = target_sv;
    }
    else
    {
        if (pri->ref - target_sv > 0)
        {
            kFactor = -1.0f;
        }
        else if (pri->ref - target_sv < 0)
        {
            kFactor = 1.0f;
        }
        else
        {
            kFactor = 0.0f;
        }
        pri->ref = pri->ref + kFactor * stepIn;
    }
}

/*封装模糊接口*/
static void compensate(float32 e, float32 ec, FUZZY_PID_t *fuzzy_d)
{
    fuzzy(e, ec, fuzzy_d);
}

/**
 * @brief 更新最大最小值
 * @param {PID_FUZZY} *self
 * @param {float32} out_min
 * @param {float32} out_max
 * @return {*}
 * @note
 */
static void _set_range(struct PID_FUZZY *self, float32 out_min, float32 out_max)
{
    self->pri.out_max = out_max;
    self->pri.out_min = out_min;
}

/**
 * @brief 设置死区
 * @param {PID_FUZZY} *self
 * @param {float32} err_dead
 * @return {*}
 * @note
 */
static void _set_err_dead(struct PID_FUZZY *self, float32 err_dead)
{
    self->pri.err_dead = err_dead;
}

static void _set_kp(struct PID_FUZZY *self, float32 kp)
{
    self->pri.kp = kp;
}

/**
 * @brief  使能积分控制
 * @param {PID_FUZZY} *self
 * @param {BOOL} enable
 * @return {*}
 * @note
 */
// static void _set_ki_enable(struct PID_FUZZY *self, BOOL enable)
// {
//     self->pri.ki_enable = enable;
// }

/**
 * @brief  使能微分控制
 * @param {PID_FUZZY} *self
 * @param {BOOL} enable
 * @return {*}
 * @note
 */
static void _set_kd_enable(struct PID_FUZZY *self, BOOL enable)
{
    self->pri.kd_enable = enable;
}

static void _set_kd(struct PID_FUZZY *self, float32 kd)
{
    self->pri.kd = kd;
}

static void _set_ki_enable(struct PID_FUZZY *self, BOOL enable)
{
    self->pri.ki_enable = enable;
}

static void _set_ki(struct PID_FUZZY *self, float32 ki)
{
    self->pri.ki = ki;
}

/*
 * Function:使能平滑控制
 * parameter:*pid需要配，PID参数结构指针，sv_range控制范围sv的范围
 * return:无
 */
static void _set_smooth_enable(struct PID_FUZZY *self, BOOL enable, float32 sv_range)
{
#if FUZZY_SUB_TYPE == PID_SUB_TYPE_POSITION
    pid_common_position_t *pri = &self->pri;
#else
    pid_common_increment_t *pri = &self->pri;
#endif
    pri->sm = enable;
    pri->sv_range = sv_range;
}

// 设置控制参数
static void _set_ctrl_prm(struct PID_FUZZY *self, float32 kp, float32 ki, float32 kd, float32 err_dead,
                          float32 out_min, float32 out_max)
{
    self->open = TRUE;
    self->fuzzy.kup = KUP;
    self->fuzzy.kui = KUI;
    self->fuzzy.kud = KUD;
#if FUZZY_SUB_TYPE == PID_SUB_TYPE_POSITION
    pid_common_position_t *pri = &self->pri;
    osel_memset((uint8_t *)pri, 0, sizeof(pid_common_position_t));
    pri->kp = kp;
    pri->ki = ki;
    pri->kd = kd;
    pri->err_dead = err_dead;
    pri->out_max = out_max;
    pri->out_min = out_min;
    pri->detach = FALSE;
    pri->sm = FALSE;
#else
    pid_common_increment_t *pri = &self->pri;
    osel_memset((uint8_t *)pri, 0, sizeof(pid_common_increment_t));
    pri->kp = kp;
    pri->ki = ki;
    pri->kd = kd;
    pri->err_dead = err_dead;
    pri->out_max = out_max;
    pri->out_min = out_min;
#endif
    pri->ki_enable = TRUE;
    if (kd > 0)
    {
        pri->kd_enable = TRUE;
    }
    else
    {
        pri->kd_enable = FALSE;
    }
}

static void _update_ctrl_prm(struct PID_FUZZY *self, float32 kp, float32 ki, float32 kd, float32 err_dead,
                             float32 out_min, float32 out_max)
{
#if FUZZY_SUB_TYPE == PID_SUB_TYPE_POSITION
    pid_common_position_t *pri = &self->pri;
    pri->kp = kp;
    pri->ki = ki;
    pri->kd = kd;
    pri->err_dead = err_dead;
    pri->out_max = out_max;
    pri->out_min = out_min;
    pri->detach = FALSE;
    pri->sm = FALSE;
#else
    pid_common_increment_t *pri = &self->pri;
    pri->kp = kp;
    pri->ki = ki;
    pri->kd = kd;
    pri->err_dead = err_dead;
    pri->out_max = out_max;
    pri->out_min = out_min;
#endif

    if (kd > 0)
    {
        pri->kd_enable = TRUE;
    }
    else
    {
        pri->kd_enable = FALSE;
    }
}

/**
 * @brief 非0时配置为积分分离+抗积分饱和PID,否则为普通抗积分饱和PID
 * @param {PID_FUZZY} *self
 * @param {float32} max_err
 * @param {BOOL} mode
 * @return {*}
 */
static void _set_cfg(struct PID_FUZZY *self, float32 max_err, BOOL mode)
{
    self->pri.err_limit = max_err;
    self->pri.detach = mode == FALSE ? FALSE : TRUE;
}

#if FUZZY_SUB_TYPE == PID_SUB_TYPE_POSITION
static float32 _PID(struct PID_FUZZY *self, float32 target, float32 feedback)
{
    float32 error = 0;
    float32 insert = 0;
    float32 ec = 0;
    float32 kd = 0;
#if INCOMPLETE_DIFFEREN == 1
    float32 thisdev = 0;
#else
    // float32 dinput = 0.0f;
#endif
    // float32 fuzzy_kp = 0.0f;
    // float32 fuzzy_ki = 0.0f;
    // float32 fuzzy_kd = 0.0f;
    float32 temp_iterm = 0.0f;
    pid_common_position_t *pri = &self->pri;

    /*获取期望值与实际值,进行偏差计算*/
    if (pri->sm == 1)
    {
        smoothSetpoint(self, target);
    }
    else
    {
        pri->ref = target;
    }

    pri->feed_back = feedback;
    error = pri->ref - pri->feed_back;
    if (fabs(error) <= pri->err_dead)
        error = 0;

    /* fuzzy control caculate */
    ec = error - pri->pre_error;
    if (self->open == TRUE)
    {
        compensate(error, ec, &self->fuzzy);
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
                temp_iterm = (pri->ki + self->fuzzy.ki) * error;
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
                temp_iterm = (pri->ki + self->fuzzy.ki) * error;
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
            temp_iterm = (pri->ki + self->fuzzy.ki) * error;
        }
    }

    pri->sum_iterm += temp_iterm;
    /* limt integral */
    if (pri->sum_iterm > pri->out_max)
    {
        pri->sum_iterm = pri->out_max;
    }
    else if (pri->sum_iterm < pri->out_min)
    {
        pri->sum_iterm = pri->out_min;
    }

#if INCOMPLETE_DIFFEREN == 1
    /*不完全微分*/
    thisdev = (pri->kd + self->fuzzy.kd) * (1.0 - pri->alpha) * (error - pri->pre_error) + pri->alpha * pri->lastdev;
    /*caculate pid out*/
    pri->out = (pri->kp + self->fuzzy.kp) * error + pri->sum_iterm * insert + thisdev;

    /*record last dev result*/
    pri->lastdev = thisdev;
#else

    if (pri->kd_enable == TRUE)
    {
        kd = pri->kd + self->fuzzy.kd;
    }
    else
    {
        kd = 0;
    }

    if (pri->ki_enable == FALSE)
    {
        insert = 0;
    }

    pri->out = (pri->kp + self->fuzzy.kp) * error + pri->sum_iterm * insert + (error - pri->pre_error) * (kd);
#endif

    pri->pre_error = error;
    /*record last feedback sensor result*/
    pri->pre_feed_back = pri->feed_back;
    /*limt pid output*/
    pri->out = RANGE(pri->out, pri->out_min, pri->out_max);
    return pri->out;
}

/**
 * @brief 复位PID积分及微分控制数据
 * @param {PID_FUZZY} *self
 * @return {*}
 */
static void _restctrl(struct PID_FUZZY *self)
{
    self->pri.pre_error = 0;
    self->pri.sum_iterm = 0;
#if INCOMPLETE_DIFFEREN == 1
    self->pri.lastdev = 0;
#endif
}

#elif FUZZY_SUB_TYPE == PID_SUB_TYPE_INCREMENT
static float32 _PID(struct PID_FUZZY *self, float32 target, float32 feedback)
{
    float fuzzy_kp = 0.0f;
    float fuzzy_ki = 0.0f;
    float fuzzy_kd = 0.0f;

    dc_t ep, ei, ed;
    dc_t inc_out;
    dc_t thisdev = 0;
    if (pri->sm == 1)
    {
        smoothSetpoint(self, target);
    }
    else
    {
        pri->ref = target;
    }
    pri->feedback = feedback;
    pri->e_0 = pri->ref - pri->feedback;
    if (fabs(pri->e_0) <= pri->err_dead)
        pri->e_0 = 0;
    ep = pri->e_0 - pri->e_1;
    ei = pri->e_0;
    ed = pri->e_0 - 2 * pri->e_1 + pri->e_2;
    if (self->open == TRUE)
    {
        compensate(pri->e_0, ep, &self->fuzzy);
    }
    /*不完全微分*/
    thisdev = (1.0 - pri->alpha) * (pri->kd + fuzzy_kd) * ed + pri->alpha * pri->lastdev;
    inc_out = (pri->kp + fuzzy_kp) * ep + (pri->ki + fuzzy_ki) * ei + thisdev;
    pri->e_2 = pri->e_1;
    pri->e_1 = pri->e_0;
    pri->lastdev = thisdev;
    //	pri->out = pri->out + inc_out;
    pri->out = inc_out;
    pri->out = RANGE(pri->out, pri->out_min, pri->out_max);
    return pri->out;
}

/**
 * @brief 复位PID积分及微分控制数据
 * @param {PID_FUZZY} *self
 * @return {*}
 */
static void _restctrl(struct PID_FUZZY *self)
{
    self->e_0 = 0;
    self->e_1 = 0;
    self->e_2 = 0;
    self->lastdev = 0;
}
#else
#error Please Define PID Controller Type
#endif

void pid_fuzzy_constructor(struct PID_FUZZY *self)
{
    self->set_ctrl_prm = _set_ctrl_prm;
    self->update_ctrl_prm = _update_ctrl_prm;
    self->set_cfg = _set_cfg;
    self->set_smooth_enable = _set_smooth_enable;
    self->set_err_dead = _set_err_dead;
    self->set_kp = _set_kp;
    self->set_ki_enable = _set_ki_enable;
    self->set_ki = _set_ki;
    self->set_kd_enable = _set_kd_enable;
    self->set_kd = _set_kd;
    self->set_range = _set_range;
    self->restctrl = _restctrl;
    self->PID = _PID;
}
