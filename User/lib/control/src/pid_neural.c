#include "pid.h"
#include <math.h>
// 设置控制参数
static void _set_ctrl_prm(struct PID_NEURAL *self, float32 minimum, float32 maximum)
{
    self->pri.setpoint = minimum; /*设定值*/

    self->pri.kcoef = 0.12; /*神经元输出比例*/
    self->pri.kp = 0.45;    /*比例学习速度*/
    self->pri.ki = 0.05;    /*积分学习速度*/
    self->pri.kd = 0;       /*微分学习速度*/

    self->pri.lasterror = 0.0;  /*前一拍偏差*/
    self->pri.preerror = 0.0;   /*前两拍偏差*/
    self->pri.result = minimum; /*PID控制器结果*/
    self->pri.output = 0.0;     /*输出值，百分比*/

    self->pri.maximum = maximum;                       /*输出值上限*/
    self->pri.minimum = minimum;                       /*输出值下限*/
    self->pri.deadband = (maximum - minimum) * 0.0005; /*死区*/

    self->pri.wp = 0.10; /*比例加权系数*/
    self->pri.wi = 0.10; /*积分加权系数*/
    self->pri.wd = 0.10; /*微分加权系数*/
}

// 设置输出参数
static void _set_out_prm(struct PID_NEURAL *self, float32 minimum, float32 maximum)
{
    self->pri.maximum = maximum;
    self->pri.minimum = minimum;
}

/*单神经元学习规则函数*/
static void NeureLearningRules(struct PID_NEURAL *self, float32 zk, float32 uk, float32 *xi)
{
    self->pri.wi = self->pri.wi + self->pri.ki * zk * uk * xi[0];
    self->pri.wp = self->pri.wp + self->pri.kp * zk * uk * xi[1];
    self->pri.wd = self->pri.wd + self->pri.kd * zk * uk * xi[2];
}

static float32 _PID(struct PID_NEURAL *self, float32 target, float32 feedback)
{
    float32 x[3];
    float32 w[3];
    float32 sabs;
    float32 error;
    float32 result;
    float32 deltaResult;
    self->pri.setpoint = target;
    error = self->pri.setpoint - feedback;
    result = self->pri.result;
    if (fabs(error) > self->pri.deadband)
    {
        x[0] = error;
        x[1] = error - self->pri.lasterror;
        x[2] = error - self->pri.lasterror * 2 + self->pri.preerror;

        sabs = fabs(self->pri.wi) + fabs(self->pri.wp) + fabs(self->pri.wd);
        w[0] = self->pri.wi / sabs;
        w[1] = self->pri.wp / sabs;
        w[2] = self->pri.wd / sabs;

        deltaResult = (w[0] * x[0] + w[1] * x[1] + w[2] * x[2]) * self->pri.kcoef;
    }
    else
    {
        deltaResult = 0;
    }

    result = result + deltaResult;
    if (result > self->pri.maximum)
    {
        result = self->pri.maximum;
    }
    if (result < self->pri.minimum)
    {
        result = self->pri.minimum;
    }
    self->pri.result = result;
    self->pri.output = self->pri.result;

    // 单神经元学习
    NeureLearningRules(self, error, result, x);

    self->pri.preerror = self->pri.lasterror;
    self->pri.lasterror = error;

    return self->pri.output;
}

void pid_neural_constructor(struct PID_NEURAL *self)
{
    self->set_ctrl_prm = _set_ctrl_prm;
    self->set_out_prm = _set_out_prm;
    self->PID = _PID;
}
