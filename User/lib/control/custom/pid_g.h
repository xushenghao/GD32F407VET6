#ifndef __PID_G_H__
#define __PID_G_H__
#include "lib.h"

typedef struct
{
    float32 ref;           /*目标*/
    float32 feed_back;     /*实际*/
    float32 pre_feed_back; /*上一次实际*/
    float32 kp;            /*比例学习速度*/
    float32 ki;            /*积分学习速度*/
    float32 kd;            /*微分学习速度*/
    float32 ki_error;      /*积分误差*/
    float32 error;         /*误差*/
    float32 pre_error;     /*前一拍偏差*/
    float32 prev_error;    /*前两拍偏差*/
    float32 err_dead;      /*死区*/
    float32 err_limit;     /*积分分离上限*/
    float32 maximum;       /*输出值的上限*/
    float32 minimum;       /*输出值的下限*/
    float32 out;           /*输出值*/
    float32 sum_iterm;     /*积分累加*/
    BOOL ki_enable;        /*积分使能*/
    BOOL kd_enable;        /*微分使能*/
    BOOL detach;
    uint16_t out_max;
    uint16_t out_min;
} g_param_t;

typedef struct PID_G
{
    /* 设置PID三个参数 */
    void (*set_ctrl_prm)(struct PID_G *self, float32 kp, float32 ki, float32 kd, float32 err_dead, float32 out_min, float32 out_max);
    /* 更新PID参数 */
    void (*update_ctrl_prm)(struct PID_G *self, float32 kp, float32 ki, float32 kd, float32 err_dead, float32 out_min, float32 out_max);
    /* 控制接口 */
    float32 (*PID)(struct PID_G *self, float32 target, float32 feedback);
    /* 更新控制区间 */
    void (*set_range)(struct PID_G *self, float32 out_min, float32 out_max);
    /* 设置积分分离 */
    void (*set_cfg)(struct PID_G *self, float32 max_err, BOOL mode);
    /* 更新kp */
    void (*set_kp)(struct PID_G *self, float32 kp);
    /* 使能ki */
    void (*set_ki_enable)(struct PID_G *self, BOOL enable);
    /* 更新ki */
    void (*set_ki)(struct PID_G *self, float32 ki);
    /* 使能kd */
    void (*set_kd_enable)(struct PID_G *self, BOOL enable);
    /* 更新kd */
    void (*set_kd)(struct PID_G *self, float32 kd);
    /* 复位PID积分及微分控制数据 */
    void (*restctrl)(struct PID_G *self);

    /*  private  */
    g_param_t pri;

} pid_g_t;

extern void pid_g_constructor(struct PID_G *self);
#endif // __PID_G_H__
