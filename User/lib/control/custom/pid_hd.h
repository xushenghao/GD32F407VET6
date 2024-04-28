#ifndef __PID_HD__
#define __PID_HD__
#include "lib.h"

#define INTEGRAL_SEPARATION 1 // 积分分离
#define INCOMPLETE_DIFFEREN 0 // 不完全微分

typedef struct
{
    float32 ref;
    float32 feed_back;
    float32 pre_feed_back;
    float32 pre_error;
    float32 ki_error; /*积分误差*/
    float32 ki_limit; /*积分分离界限*/
    float32 kp_limit; /*变积分界限*/
    float32 alpha;    /*不完全微分系数*/
    float32 err;
    float32 sum_iterm;
    float32 kp;
    float32 kp_small; /*在接近稳态时的Kp*/
    float32 kp_big;   /*在大范围时的Kp*/
    float32 ki;
    float32 kd;
    float32 err_limit;
    BOOL detach;
    float32 err_dead;
#if INCOMPLETE_DIFFEREN == 1
    dc_t alpha;
    dc_t lastdev;
#endif
    float32 out;
    float32 pre_out;
    float32 out_max;
    float32 out_min;
    BOOL sm;
    float32 sv_range;
} pid_hd_position_t; // 位置式PID

typedef struct PID_HD
{
    /*  设置PID三个参数  */
    void (*set_ctrl_prm_position)(struct PID_HD *self, float32 kp, float32 ki, float32 kd);
    /*  设置输出范围  */
    void (*set_out_prm_position)(struct PID_HD *self, float32 maximum, float32 minimum);

    /*  控制接口  */
    float32 (*pid_position)(struct PID_HD *self, float32 err);

    // 自定义参数
    /*  实际值与目标值之间的误差  */
    float32 err;
    /*  输出值  */
    float32 out;
    /*  private  */
    struct
    {
        pid_hd_position_t position;
    } pri_u;
} pid_hd_t;

extern void pid_hd_constructor(struct PID_HD *self);

#endif // __PID_HD__
