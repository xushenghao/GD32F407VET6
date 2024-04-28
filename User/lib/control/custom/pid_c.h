#ifndef __PID_C_H__
#define __PID_C_H__
#include "lib.h"

typedef struct PID_C
{
    /*  设置PID三个参数  */
    void (*set_ctrl_prm)(struct PID_C *self, float32 kp, float32 ki, float32 kd, float32 out_min, float32 out_max);
    /*  控制接口  */
    float32 (*PID)(struct PID_C *self, float32 target, float32 feedback);

    // 自定义参数
    /*  实际值与目标值之间的误差  */
    float32 err;
    /*  输出值  */
    float32 out;
    /*  private  */
    struct
    {
        float32 kp;         /*比例学习速度*/
        float32 ki;         /*积分学习速度*/
        float32 kd;         /*微分学习速度*/
        float32 ki_error;   /*积分误差*/
        float32 last_error; /*前一拍偏差*/
        float32 prev_error; /*前两拍偏差*/
        float32 deadband;   /*死区*/
        float32 maximum;    /*输出值的上限*/
        float32 minimum;    /*输出值的下限*/
    } pri;
} pid_c_t;

extern void pid_c_constructor(struct PID_C *self);

#endif // __PID_C_H__
