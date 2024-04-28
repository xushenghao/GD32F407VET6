#ifndef __PID_X_H__
#define __PID_X_H__
#include "lib.h"

/*定义PID对象类型*/
typedef struct CLASSIC
{
    float32 *pPV; // 测量值指针
    float32 *pSV; // 设定值指针
    float32 *pMV; // 输出值指针
    BOOL *pMA;    // 手自动操作指针

    float32 out;       // 输出值
    float32 setpoint;  // 设定值
    float32 lasterror; // 前一拍偏差
    float32 preverror; // 前两拍偏差
    float32 max;       // 输出值上限
    float32 min;       // 输出值下限

    uint16_t flag; // 状态标志位

    float32 pKp; // 比例系数
    float32 pKi; // 积分系数
    float32 pKd; // 微分系数

    float32 nKp; // 比例系数
    float32 nKi; // 积分系数
    float32 nKd; // 微分系数

    BOOL direct; // 正反作用
    BOOL sm;     // 设定值平滑
    BOOL cas;    // 串级设定
    BOOL pac;    // 输出防陡变
    BOOL kd_e;   // 微分使能
} CLASSICPID;

// 定义整定参数
typedef struct
{
    uint8_t tuneEnable : 2;     // 整定与PID控制开关，0：PID控制；1：参数整定；2：整定失败
    uint8_t preEnable : 2;      // 预处理使能，在开始整定前置位
    uint8_t initialStatus : 1;  // 记录开始整定前偏差的初始状态
    uint8_t outputStatus : 1;   // 记录输出的初始状态，0允许上升过零计数；1允许下降过零计数
    uint8_t controllerType : 2; // 控制器类型：0，P控制器；1，PI控制器；2，PID控制器

    uint8_t zeroAcrossCounter; // 过零点计数器，每次输出改变加1，比实际过零次数多1
    uint8_t riseLagCounter;    // 上升迟滞时间计数器
    uint8_t fallLagCounter;    // 下降迟滞时间计数器

    uint16_t tunePeriod; // 整定采样周期
    uint32_t tuneTimer;  // 整定计时器
    uint32_t startTime;  // 记录波形周期起始时间
    uint32_t endTime;    // 记录波形周期结束时间

    float32 outputStep; // 输出阶跃d
    float32 maxPV;      // 振荡波形中测量值的最大值
    float32 minPV;      // 振荡波形中测量值的最小值
} CLASSIC_AUTOTUNE;

typedef struct PID_X
{
    /*  控制接口  */
    float32 (*PID)(struct PID_X *self, float32 target, float32 feedback);
    uint8_t (*AUTO_TUNE)(struct PID_X *self);
    /*  private  */
    CLASSICPID pri;
    CLASSIC_AUTOTUNE tune;
} pid_x_t;

extern void pid_x_constructor(struct PID_X *self);
#endif // __PID_X_H__
