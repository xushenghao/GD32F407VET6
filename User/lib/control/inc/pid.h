#ifndef __PID_H__
#define __PID_H__
#include "lib.h"
#include "pid_auto_tune.h"

#include "pid_c.h"
#include "pid_g.h"
#include "pid_x.h"
#include "pid_zh.h"
#include "pid_hd.h"

typedef enum
{
    // PID自整定
    PID_TYPE_AUTO_TUNE,
    // 通用PID
    PID_TYPE_COMMON,
    // 神经PID
    PID_TYPE_NEURAL,
    // 模糊PID
    PID_TYPE_FUZZY,

    // 以下是自定义PID

    // cj PID
    PID_TYPE_CUSTOM_CAO,
    // gp jPID
    PID_TYPE_CUSTOM_GAO,
    // xsh PID
    PID_TYPE_CUSTOM_XU,
    // zxm PID
    PID_TYPE_CUSTOM_ZHANG,
    // hangdian PID
    PID_TYPE_CUSTOM_HANGDIAN,
} pid_type_e;

typedef enum
{
    // 位置式
    PID_SUB_TYPE_POSITION,
    // 增量式
    PID_SUB_TYPE_INCREMENT,
} pid_sub_type_e;

#define FUZZY_SUB_TYPE PID_SUB_TYPE_POSITION
#define INCOMPLETE_DIFFEREN 0 // 不完全微分

#pragma pack(1)
typedef struct
{
    float32 ref;
    float32 feed_back;
    float32 pre_feed_back;
    float32 pre_error;
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
} pid_common_position_t; // 位置式PID

typedef struct
{
    float32 ref;      // 目标设定值
    float32 feedback; // 传感器采集值
    float32 out;      // PID计算结果
    float32 kp;
    float32 ki;
    float32 kd;
    float32 e_0; // 当前误差
    float32 e_1; // 上一次误差
    float32 e_2; // 上上次误差
    float32 alpha;
    float32 lastdev;
    float32 err_dead;
    float32 out_max; // 输出限幅
    float32 out_min; // 输出限幅
    BOOL sm;
    float32 sv_range;
} pid_common_increment_t; // 增量式PID
#pragma pack()

typedef struct PID_COMMON
{
    pid_sub_type_e type;
    /*  设置PID三个参数  */
    void (*set_ctrl_prm)(struct PID_COMMON *self, float32 kp, float32 ki, float32 kd);
    /*  设置积分范围  */
    void (*set_integral_prm)(struct PID_COMMON *self, float32 integral_up, float32 integral_low);

    /*  控制接口  */
    float32 (*PID)(struct PID_COMMON *self, float32 err);

    /*  in  value  */
    float32 err;
    /*  out  value  */
    float32 out;

    union
    {
        pid_common_position_t position;
        pid_common_increment_t increment;
    } pri_u;

} pid_common_t; // 通用PID

typedef struct PID_NEURAL
{
    pid_sub_type_e type;
    /*  设置PID三个参数  */
    void (*set_ctrl_prm)(struct PID_NEURAL *self, float32 minimum, float32 maximum);
    /*  设置输出范围  */
    void (*set_out_prm)(struct PID_NEURAL *self, float32 minimum, float32 maximum);
    /*  控制接口  */
    float32 (*PID)(struct PID_NEURAL *self, float32 target, float32 feedback);

    struct
    {
        float32 setpoint;  /*设定值*/
        float32 kcoef;     /*神经元输出比例*/
        float32 kp;        /*比例学习速度*/
        float32 ki;        /*积分学习速度*/
        float32 kd;        /*微分学习速度*/
        float32 lasterror; /*前一拍偏差*/
        float32 preerror;  /*前两拍偏差*/
        float32 deadband;  /*死区*/
        float32 result;    /*输出值*/
        float32 output;    /*百分比输出值*/
        float32 maximum;   /*输出值的上限*/
        float32 minimum;   /*输出值的下限*/
        float32 wp;        /*比例加权系数*/
        float32 wi;        /*积分加权系数*/
        float32 wd;        /*微分加权系数*/
    } pri;
} pid_neural_t; // 神经PID

typedef struct
{
    float32 kp;
    float32 ki;
    float32 kd;

    float32 kup;
    float32 kui;
    float32 kud;
} FUZZY_PID_t;

// 模糊PID
typedef struct PID_FUZZY
{
    /*  设置PID三个参数  */
    void (*set_ctrl_prm)(struct PID_FUZZY *self, float32 kp, float32 ki, float32 kd, float32 err_dead,
                         float32 out_min, float32 out_max); // 设置PID参数
    void (*update_ctrl_prm)(struct PID_FUZZY *self, float32 kp, float32 ki, float32 kd, float32 err_dead,
                            float32 out_min, float32 out_max);                        // 更新PID参数
    void (*set_range)(struct PID_FUZZY *self, float32 out_min, float32 out_max);      // 更新最大最小值
    void (*set_cfg)(struct PID_FUZZY *self, float32 max_err, BOOL mode);              // 配置PID模式,默认不使用积分分离
    void (*set_smooth_enable)(struct PID_FUZZY *self, BOOL enable, float32 sv_range); // 设置平滑范围

    void (*set_err_dead)(struct PID_FUZZY *self, float32 err_dead); // 设置死区
    void (*set_kp)(struct PID_FUZZY *self, float32 kp);
    void (*set_ki_enable)(struct PID_FUZZY *self, BOOL enable);
    void (*set_ki)(struct PID_FUZZY *self, float32 ki);
    // 微分开启使能
    void (*set_kd_enable)(struct PID_FUZZY *self, BOOL enable);
    void (*set_kd)(struct PID_FUZZY *self, float32 kd);
    void (*restctrl)(struct PID_FUZZY *self); // 复位PID积分及微分控制数据
    /*  控制接口  */
    float32 (*PID)(struct PID_FUZZY *self, float32 target, float32 feedback);

#if FUZZY_SUB_TYPE == PID_SUB_TYPE_POSITION
    pid_common_position_t pri;
#else
    pid_common_increment_t pri;
#endif

    BOOL open; // 是否使用模糊PID控制

    FUZZY_PID_t fuzzy;

} pid_fuzzy_t; // 模糊PID

// PID
typedef struct
{
    pid_type_e type;
    union
    {
        pid_common_t common;
        pid_neural_t neural;
        pid_fuzzy_t fuzzy;

        // 自定义PID
        pid_c_t cao;
        pid_g_t gao;
        pid_x_t xu;
        pid_zh_t zhang;
        pid_hd_t hd;
    } pid_u;
    pid_auto_tune_t auto_tune;
} pid_t;

// PID控制
extern void pid_constructor(pid_t *self);

// private
// 神经元PID
extern void pid_neural_constructor(struct PID_NEURAL *self);
// 模糊PID
extern void pid_fuzzy_constructor(struct PID_FUZZY *self);

#endif
