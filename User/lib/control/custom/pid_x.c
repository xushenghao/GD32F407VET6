#include "pid_x.h"
#include "math.h"
#define LAG_PHASE (6) // 迟滞相位，单位：拍

#ifndef PI
#define PI 3.14159265358979f
#endif
// 注1：自适应模糊pid最重要的就是论域的选择，要和你应该控制的对象相切合
// 注2：以下各阀值、限幅值、输出值均需要根据具体的使用情况进行更改
// 注3：因为我的控制对象惯性比较大，所以以下各部分取值较小
// 论域e:[-5,5]  ec:[-0.5,0.5]

// 误差的阀值，小于这个数值的时候，不做PID调整，避免误差较小时频繁调节引起震荡
#define Emin 0.3f
#define Emid 1.0f
#define Emax 5.0f
// 调整值限幅，防止积分饱和
#define Umax 1
#define Umin -1

#define NB 0
#define NM 1
#define NS 2
#define ZO 3
#define PS 4
#define PM 5
#define PB 6

int32_t kp[7][7] = {{PB, PB, PM, PM, PS, ZO, ZO},
                    {PB, PB, PM, PS, PS, ZO, ZO},
                    {PM, PM, PM, PS, ZO, NS, NS},
                    {PM, PM, PS, ZO, NS, NM, NM},
                    {PS, PS, ZO, NS, NS, NM, NM},
                    {PS, ZO, NS, NM, NM, NM, NB},
                    {ZO, ZO, NM, NM, NM, NB, NB}};

int32_t kd[7][7] = {{PS, NS, NB, NB, NB, NM, PS},
                    {PS, NS, NB, NM, NM, NS, ZO},
                    {ZO, NS, NM, NM, NS, NS, ZO},
                    {ZO, NS, NS, NS, NS, NS, ZO},
                    {ZO, ZO, ZO, ZO, ZO, ZO, ZO},
                    {PB, NS, PS, PS, PS, PS, PB},
                    {PB, PM, PM, PM, PS, PS, PB}};

int32_t ki[7][7] = {{NB, NB, NM, NM, NS, ZO, ZO},
                    {NB, NB, NM, NS, NS, ZO, ZO},
                    {NB, NM, NS, NS, ZO, PS, PS},
                    {NM, NM, NS, ZO, PS, PM, PM},
                    {NM, NS, ZO, PS, PS, PM, PB},
                    {ZO, ZO, PS, PS, PM, PB, PB},
                    {ZO, ZO, PS, PM, PM, PB, PB}};

static float32 ec; // 误差变化率
/**************求隶属度（三角形）***************/
float32 FTri(float32 x, float32 a, float32 b, float32 c) // FuzzyTriangle
{
    if (x <= a)
        return 0;
    else if ((a < x) && (x <= b))
        return (x - a) / (b - a);
    else if ((b < x) && (x <= c))
        return (c - x) / (c - b);
    else if (x > c)
        return 0;
    else
        return 0;
}
/*****************求隶属度（梯形左）*******************/
float32 FTraL(float32 x, float32 a, float32 b) // FuzzyTrapezoidLeft
{
    if (x <= a)
        return 1;
    else if ((a < x) && (x <= b))
        return (b - x) / (b - a);
    else if (x > b)
        return 0;
    else
        return 0;
}
/*****************求隶属度（梯形右）*******************/
float32 FTraR(float32 x, float32 a, float32 b) // FuzzyTrapezoidRight
{
    if (x <= a)
        return 0;
    if ((a < x) && (x < b))
        return (x - a) / (b - a);
    if (x >= b)
        return 1;
    else
        return 1;
}
/****************三角形反模糊化处理**********************/
float32 uFTri(float32 x, float32 a, float32 b, float32 c)
{
    float32 y, z;
    z = (b - a) * x + a;
    y = c - (c - b) * x;
    return (y + z) / 2;
}
/*******************梯形（左）反模糊化***********************/
float32 uFTraL(float32 x, float32 a, float32 b)
{
    return b - (b - a) * x;
}
/*******************梯形（右）反模糊化***********************/
float32 uFTraR(float32 x, float32 a, float32 b)
{
    return (b - a) * x + a;
}
/**************************求交集****************************/
float32 fand(float32 a, float32 b)
{
    return (a < b) ? a : b;
}
/**************************求并集****************************/
float32 forr(float32 a, float32 b)
{
    return (a < b) ? b : a;
}

static float32 _PID(struct PID_X *self, float32 target, float32 feedback)
{
    float32 pwm_var; // pwm调整量
    float32 iError;  // 当前误差
    float32 set, input;
    CLASSICPID *pri = &self->pri;
    // 计算隶属度表
    float32 es[7], ecs[7], e;
    float32 form[7][7];
    int i = 0, j = 0;
    int MaxX = 0, MaxY = 0;

    // 记录隶属度最大项及相应推理表的p、i、d值
    float32 lsd;
    int temp_p, temp_d, temp_i;
    float32 detkp, detkd, detki; // 推理后的结果

    // 输入格式的转化及偏差计算
    set = target;
    input = feedback;
    iError = set - input; // 偏差

    e = iError;
    ec = iError - pri->lasterror;

    // 当温度差的绝对值小于Emax时，对pid的参数进行调整
    if (fabs(iError) <= Emax)
    {
        // 计算iError在es与ecs中各项的隶属度
        es[NB] = FTraL(e * 5, -3, -1); // e
        es[NM] = FTri(e * 5, -3, -2, 0);
        es[NS] = FTri(e * 5, -3, -1, 1);
        es[ZO] = FTri(e * 5, -2, 0, 2);
        es[PS] = FTri(e * 5, -1, 1, 3);
        es[PM] = FTri(e * 5, 0, 2, 3);
        es[PB] = FTraR(e * 5, 1, 3);

        ecs[NB] = FTraL(ec * 30, -3, -1); // ec
        ecs[NM] = FTri(ec * 30, -3, -2, 0);
        ecs[NS] = FTri(ec * 30, -3, -1, 1);
        ecs[ZO] = FTri(ec * 30, -2, 0, 2);
        ecs[PS] = FTri(ec * 30, -1, 1, 3);
        ecs[PM] = FTri(ec * 30, 0, 2, 3);
        ecs[PB] = FTraR(ec * 30, 1, 3);

        // 计算隶属度表，确定e和ec相关联后表格各项隶属度的值
        for (i = 0; i < 7; i++)
        {
            for (j = 0; j < 7; j++)
            {
                form[i][j] = fand(es[i], ecs[j]);
            }
        }

        // 取出具有最大隶属度的那一项
        for (i = 0; i < 7; i++)
        {
            for (j = 0; j < 7; j++)
            {
                if (form[MaxX][MaxY] < form[i][j])
                {
                    MaxX = i;
                    MaxY = j;
                }
            }
        }
        // 进行模糊推理，并去模糊
        lsd = form[MaxX][MaxY];
        temp_p = kp[MaxX][MaxY];
        temp_d = kd[MaxX][MaxY];
        temp_i = ki[MaxX][MaxY];

        if (temp_p == NB)
            detkp = uFTraL(lsd, -0.3, -0.1);
        else if (temp_p == NM)
            detkp = uFTri(lsd, -0.3, -0.2, 0);
        else if (temp_p == NS)
            detkp = uFTri(lsd, -0.3, -0.1, 0.1);
        else if (temp_p == ZO)
            detkp = uFTri(lsd, -0.2, 0, 0.2);
        else if (temp_p == PS)
            detkp = uFTri(lsd, -0.1, 0.1, 0.3);
        else if (temp_p == PM)
            detkp = uFTri(lsd, 0, 0.2, 0.3);
        else if (temp_p == PB)
            detkp = uFTraR(lsd, 0.1, 0.3);

        if (temp_d == NB)
            detkd = uFTraL(lsd, -3, -1);
        else if (temp_d == NM)
            detkd = uFTri(lsd, -3, -2, 0);
        else if (temp_d == NS)
            detkd = uFTri(lsd, -3, 1, 1);
        else if (temp_d == ZO)
            detkd = uFTri(lsd, -2, 0, 2);
        else if (temp_d == PS)
            detkd = uFTri(lsd, -1, 1, 3);
        else if (temp_d == PM)
            detkd = uFTri(lsd, 0, 2, 3);
        else if (temp_d == PB)
            detkd = uFTraR(lsd, 1, 3);

        if (temp_i == NB)
            detki = uFTraL(lsd, -0.06, -0.02);
        else if (temp_i == NM)
            detki = uFTri(lsd, -0.06, -0.04, 0);
        else if (temp_i == NS)
            detki = uFTri(lsd, -0.06, -0.02, 0.02);
        else if (temp_i == ZO)
            detki = uFTri(lsd, -0.04, 0, 0.04);
        else if (temp_i == PS)
            detki = uFTri(lsd, -0.02, 0.02, 0.06);
        else if (temp_i == PM)
            detki = uFTri(lsd, 0, 0.04, 0.06);
        else if (temp_i == PB)
            detki = uFTraR(lsd, 0.02, 0.06);

        // pid三项系数的修改
        pri->pKp += detkp;
        pri->pKi += detki;
        if (pri->kd_e)
        {
            pri->pKd += detkd;
        }
        else
        {
            pri->pKd = 0; // 取消微分作用
        }

        // 对Kp,Ki进行限幅
        if (pri->pKp < 0)
        {
            pri->pKp = 0;
        }
        if (pri->pKi < 0)
        {
            pri->pKi = 0;
        }

        // 计算新的K1,nKi,nKd
        pri->nKp = pri->pKp + pri->pKi + pri->pKd;
        pri->nKi = -(pri->pKp + 2 * pri->pKd);
        pri->nKd = pri->pKd;
    }

    if (iError > Emax)
    {
        pri->out = pri->max;
        pwm_var = 0;
        pri->flag = 1; // 设定标志位，如果误差超过了门限值，则认为当控制量第一次到达给定值时，应该采取下面的 抑制超调 的措施
    }
    else if (iError < -Emax)
    {
        pri->out = pri->min;
        pwm_var = 0;
    }
    else if (fabsf(iError) <= Emin)
    {
        pwm_var = 0;
    }
    else
    {
        if (iError < Emid && pri->flag == 1) // 第一次超过(设定值-Emid(-0.08)摄氏度)，是输出为零，防止超调，也可以输出其他值，不至于太小而引起震荡
        {
            pri->out = 0;
            pri->flag = 0;
        }
        else if (-iError > Emid) // 超过(设定+Emid(+0.08)摄氏度)
        {
            pwm_var = -1;
        }
        else
        {
            // 增量计算
            pwm_var = (pri->nKp * iError             // e[k]
                       + pri->nKi * pri->lasterror   // e[k-1]
                       + pri->nKd * pri->preverror); // e[k-2]
        }
        if (pwm_var >= Umax)
            pwm_var = Umax; // 调整值限幅，防止积分饱和
        if (pwm_var <= Umin)
            pwm_var = Umin; // 调整值限幅，防止积分饱和
    }
    pri->preverror = pri->lasterror;
    pri->lasterror = iError;

    pri->out += pwm_var; // 调整PWM输出

    if (pri->out > pri->max)
        pri->out = pri->max; // 输出值限幅
    if (pri->out < pri->min)
        pri->out = pri->min; // 输出值限幅

    return pri->out;
}

/*整定开始前的预处理，判断状态及初始化变量*/
static void tune_pretreatment(struct PID_X *self)
{
    CLASSIC_AUTOTUNE *tune = &self->tune;
    CLASSICPID *vPID = &self->pri;

    tune->tuneTimer = 0;
    tune->startTime = 0;
    tune->endTime = 0;
    tune->outputStep = 100;

    if (*vPID->pSV >= *vPID->pPV)
    {
        tune->initialStatus = 1;
        tune->outputStatus = 0;
    }
    else
    {
        tune->initialStatus = 0;
        tune->outputStatus = 1;
    }
    tune->tuneEnable = 1;
    tune->preEnable = 0;
    tune->zeroAcrossCounter = 0;
    tune->riseLagCounter = 0;
    tune->fallLagCounter = 0;
}

/*计算PID参数值*/
static void calculation_parameters(struct PID_X *self)
{
    CLASSIC_AUTOTUNE *tune = &self->tune;
    CLASSICPID *vPID = &self->pri;
    float32 kc = 0.0;
    float32 tc = 0.0;
    float32 zn[3][3] = {{0.5, 100000.0, 0.0}, {0.45, 0.8, 0.0}, {0.6, 0.5, 0.125}};

    tc = (tune->endTime - tune->startTime) * tune->tunePeriod / 1000.0;
    kc = (8.0 * tune->outputStep) / (PI * (tune->maxPV - tune->minPV));

    vPID->pKp = zn[tune->controllerType][0] * kc;                                  // 比例系数
    vPID->pKi = vPID->pKp * tune->tunePeriod / (zn[tune->controllerType][1] * tc); // 积分系数
    vPID->pKd = vPID->pKp * zn[tune->controllerType][2] * tc / tune->tunePeriod;   // 微分系数
}

/**
 * @brief   自整定函数
 * @param {PID_X} *self
 * @return {*}
 * @note  成员变量tuneEnable、preEnable和controllerType需要提前赋值。tuneEnable变量值为0时是使用PID控制器，而tuneEnable变量值为1时是开启整定过程，当tuneEnable变量值为2时是指示整定失败。preEnable变量在整定前赋值为1，表示先做预处理。而controllerType则根据所整定的控制器的类型来定，主要用于参数的计算。
 */
static uint8_t _auto_tune(struct PID_X *self)
{
    CLASSIC_AUTOTUNE *tune = &self->tune;
    CLASSICPID *vPID = &self->pri;
    /*整定开始前的预处理，只执行一次*/
    if (tune->preEnable == 1)
    {
        tune_pretreatment(self);
    }

    uint32_t tuneDuration = 0;
    tune->tuneTimer++;
    tuneDuration = (tune->tuneTimer * tune->tunePeriod) / 1000;
    if (tuneDuration > (10 * 60)) // 整定过程持续超过10分钟，未能形成有效振荡，整定失败
    {
        tune->tuneEnable = 2;
        tune->preEnable = 1;
        return tune->tuneEnable;
    }

    if (*vPID->pSV >= *vPID->pPV) // 设定值大于测量值，则开执行单元
    {
        tune->riseLagCounter++;
        tune->fallLagCounter = 0;

        if (tune->riseLagCounter > LAG_PHASE)
        {
            *vPID->pMV = vPID->max;
            if (tune->outputStatus == 0)
            {
                tune->outputStatus = 1;
                tune->zeroAcrossCounter++;

                if (tune->zeroAcrossCounter == 3)
                {
                    tune->startTime = tune->tuneTimer;
                }
            }
        }
    }
    else
    {
        tune->riseLagCounter = 0;
        tune->fallLagCounter++;

        if (tune->fallLagCounter > LAG_PHASE)
        {
            *vPID->pMV = vPID->min;
            if (tune->outputStatus == 1)
            {
                tune->outputStatus = 0;
                tune->zeroAcrossCounter++;

                if (tune->zeroAcrossCounter == 3)
                {
                    tune->startTime = tune->tuneTimer;
                }
            }
        }
    }

    if (tune->zeroAcrossCounter == 3) // 已经两次过零，可以记录波形数据
    {
        if (tune->initialStatus == 1) // 初始设定值大于测量值，则区域3出现最小值
        {
            if (*vPID->pPV < tune->minPV)
            {
                tune->minPV = *vPID->pPV;
            }
        }
        else if (tune->initialStatus == 0) // 初始设定值小于测量值，则区域3出现最大值
        {
            if (*vPID->pPV > tune->maxPV)
            {
                tune->maxPV = *vPID->pPV;
            }
        }
    }
    else if (tune->zeroAcrossCounter == 4) // 已经三次过零，记录另半波的数据
    {
        if (tune->initialStatus == 1) // 初始设定值大于测量值，则区域4出现最大值
        {
            if (*vPID->pPV > tune->maxPV)
            {
                tune->maxPV = *vPID->pPV;
            }
        }
        else if (tune->initialStatus == 0) // 初始设定值小于测量值，则区域4出现最小值
        {
            if (*vPID->pPV < tune->minPV)
            {
                tune->minPV = *vPID->pPV;
            }
        }
    }
    else if (tune->zeroAcrossCounter == 5) // 已经四次过零，振荡已形成可以整定参数
    {
        calculation_parameters(self);

        tune->tuneEnable = 0;
        tune->preEnable = 1;
    }

    return tune->tuneEnable;
}

void pid_x_constructor(struct PID_X *self)
{
    self->PID = _PID;
    self->AUTO_TUNE = _auto_tune;
    self->pri.flag = 0;
    self->pri.out = 0;
    self->tune.preEnable = 1;
}
