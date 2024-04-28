/***
 * @Author:
 * @Date: 2023-07-24 11:17:55
 * @LastEditors: xxx
 * @LastEditTime: 2023-07-24 11:19:06
 * @Description:pid自动调参，构建闭环回路 确定稳定极限   确定两个参数  极限值KP和震荡周期
 * @email:
 * @Copyright (c) 2023 by xxx, All Rights Reserved.
 */
#ifndef __PID_AUTO_TUNE_H__
#define __PID_AUTO_TUNE_H__
#include "lib.h"

typedef struct PID_AUTO_TUNE
{
    // public:
    void (*set_ctrl_prm)(struct PID_AUTO_TUNE *self, float32 *input, float32 *output);
    int32_t (*runtime)(struct PID_AUTO_TUNE *self);
    void (*set_output_step)(struct PID_AUTO_TUNE *self, int32_t step);
    void (*set_control_type)(struct PID_AUTO_TUNE *self, int32_t type);
    void (*set_noise_band)(struct PID_AUTO_TUNE *self, int32_t band);
    void (*set_look_back)(struct PID_AUTO_TUNE *self, int32_t n);
    float32 (*get_kp)(struct PID_AUTO_TUNE *self);
    float32 (*get_ki)(struct PID_AUTO_TUNE *self);
    float32 (*get_kd)(struct PID_AUTO_TUNE *self);
    // private:
    struct
    {
        BOOL isMax, isMin; // 运算中出现最大、最小值标志
        float32 *input, *output;
        float32 setpoint;    // 反向控制判断值，这个值需要根据对象的实际工作值确定！是通过第一次启动时对应的输入值带入的。
        int32_t noiseBand;   // 判断回差，类似于施密特触发器，实际控制反向的比较值是 setpoint + noiseBand 或 setpoint - noiseBand
        int32_t controlType; // 计算 PID 参数时，选择 PI 或 PID 模式，输出 Kp Ki，或 Kp、Ki、Kd
        BOOL running;
        uint32_t peak1, peak2, lastTime; // 峰值对应的时间
        int32_t sampleTime;
        int32_t nLookBack;
        int32_t peakType;
        // double lastInputs[101];							// 保存的历史输入值， 最多存前 100 次
        int32_t lastInputs[51]; // 保存的历史输入值, 改为 50 次。 by shenghao.xu
        int32_t peaks[13];      // 保存的历史峰值，最多存前 12 次，对应 6个最大、6个最小。20221124 by Embedream
        int32_t peakCount;      // 峰值计数
        int32_t peakPeriod[7];  // 保存前 6 次的最大值间隔时间 by shenghao.xu
        int32_t peakMaxCount;   // 最大峰值计数 by shenghao.xu
        BOOL justchanged;
        // BOOL justevaled;                    // 此标志没有使用
        // int32_t absMax, absMin;								  // 整个过程中采集的输入最大值、最小值
        int32_t oStep;       // 这个值是用于计算控制高低值的，以 outputStart 为中值，输出高值用 outputStart + oStep， 输出低值用 outputStart - oStep
        float32 outputStart; // 输出控制的基础值，这个需要结合对象特征确定，此值也是通过第一次启动时对应的输出值带入的。
        float32 Ku, Pu;
    } pri;

} pid_auto_tune_t;

extern void pid_auto_tune_constructor(struct PID_AUTO_TUNE *self);
#endif // __PID_AUTO_TUNE_H__
