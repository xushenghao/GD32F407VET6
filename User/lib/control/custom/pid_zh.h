#ifndef __PID_ZH_H__
#define __PID_ZH_H__
#include "lib.h"
#include "pid_auto_tune.h"

typedef struct
{
    float32 ref;
    float32 feed_back;
    float32 pre_feed_back;
    float32 pre_error;
    float32 sum_ref;
    float32 sum_iterm;
    float32 kp;
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
    float32 out_max;
    float32 out_min;
    float32 sv_range;
    BOOL sm;
    BOOL ki_enable;
    BOOL kd_enable;
} pid_zh_position_t; // 位置式PID

typedef struct
{
    float32 kp;
    float32 ki;
    float32 kd;

    float32 kup;
    float32 kui;
    float32 kud;
} FUZZY_PID_ZH_t;

// 模糊PID
typedef struct PID_FUZZY_ZH
{
    /*  设置PID三个参数  */
    void (*set_ctrl_prm)(struct PID_FUZZY_ZH *self, float32 kp, float32 ki, float32 kd, float32 err_dead,
                         float32 out_min, float32 out_max); // 设置PID参数
    void (*update_ctrl_prm)(struct PID_FUZZY_ZH *self, float32 kp, float32 ki, float32 kd, float32 err_dead,
                            float32 out_min, float32 out_max);                           // 更新PID参数
    void (*set_range)(struct PID_FUZZY_ZH *self, float32 out_min, float32 out_max);      // 更新最大最小值
    void (*set_cfg)(struct PID_FUZZY_ZH *self, float32 max_err, BOOL mode);              // 配置PID模式,默认不使用积分分离
    void (*set_smooth_enable)(struct PID_FUZZY_ZH *self, BOOL enable, float32 sv_range); // 设置平滑范围
    // void (*set_ki_enable)(struct PID_FUZZY *self, BOOL enable);
    // 微分开启使能
    void (*set_kd_enable)(struct PID_FUZZY_ZH *self, BOOL enable);
    void (*restctrl)(struct PID_FUZZY_ZH *self); // 复位PID积分及微分控制数据
    /*  控制接口  */
    float32 (*PID)(struct PID_FUZZY_ZH *self, float32 target, float32 feedback);

    pid_zh_position_t pri;

    BOOL open; // 是否使用模糊PID控制

    FUZZY_PID_ZH_t fuzzy;

} pid_zh_t; // 模糊PID

extern void pid_zh_constructor(struct PID_FUZZY_ZH *self);
#endif
