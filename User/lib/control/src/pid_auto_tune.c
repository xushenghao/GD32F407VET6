#include "pid_auto_tune.h"
#include "sys.h"

/*
  设置峰值回溯时间,单位 0.1 秒，最小 0.2秒， 最大 4 秒
*/
static void set_look_backsec(pid_auto_tune_t *self, int32_t value)
{
    if (value < 2)
        value = 2;
    if (value > 40)
        value = 40;

    if (value < 40)
    {
        self->pri.nLookBack = 12;           // 按目前实际周期约300ms、采样周期 10ms 考虑，一个周期只有 30 点，回溯 12 点即可。
        self->pri.sampleTime = value * 10; // 改为 Value*100 ms， 20、30、40 ~ 200ms
    }
    else
    {
        self->pri.nLookBack = 50 + value;
        self->pri.sampleTime = 4000;
    }
}

static void _set_ctrl_prm(struct PID_AUTO_TUNE *self, float32 *input, float32 *output)
{
    self->pri.input = input;
    self->pri.output = output;
    self->pri.controlType = 0; // 默认为 PI 模式
    self->pri.noiseBand = 1;
    self->pri.running = FALSE;
    self->pri.oStep = 1;
    set_look_backsec(self, 1);
    self->pri.lastTime = sys_millis();
}

static void _set_noise_band(struct PID_AUTO_TUNE *self, int32_t value)
{
    self->pri.noiseBand = value;
}

static void _set_output_step(struct PID_AUTO_TUNE *self, int32_t value)
{
    self->pri.oStep = value;
}

// * Determies if the tuning parameters returned will be PI (D=0)
//   or PID.  (0=PI, 1=PID)
static void _set_control_type(struct PID_AUTO_TUNE *self, int32_t value)
{
    self->pri.controlType = value;
}

static void _set_look_back(struct PID_AUTO_TUNE *self, int32_t value)
{
    set_look_backsec(self, value);
}

static float32 _get_kp(struct PID_AUTO_TUNE *self)
{
    float32 kp = self->pri.controlType == 1 ? 0.6f * self->pri.Ku : 0.4f * self->pri.Ku;
    return kp;
}

static float32 _get_ki(struct PID_AUTO_TUNE *self)
{
    float32 ki = self->pri.controlType == 1 ? 1.2f * self->pri.Ku / self->pri.Pu : 0.48f * self->pri.Ku / self->pri.Pu;
    return ki;
}

static float32 _get_kd(struct PID_AUTO_TUNE *self)
{
    return self->pri.controlType == 1 ? 0.075f * self->pri.Ku * self->pri.Pu : 0;
}

/**
 * @brief 修改返回值，0 - 执行计算，未完成整定， 1 - 执行计算，完成整定过程， 2 - 采样时间未到
 * @return {*}
 */
static int32_t _runtime(struct PID_AUTO_TUNE *self)
{
    int32_t i, iSum;

    uint32_t now = sys_millis();
    if ((now - self->pri.lastTime) < ((uint32_t)self->pri.sampleTime))
    {
        return 2; // 原来返回值为 FALSE  不符合函数定义，也无法区分，改为 2，by shenghao.xu
    }

    // 开始整定计算
    self->pri.lastTime = now;
    float32 refVal = *(self->pri.input);
    if (FALSE == self->pri.running) // 首次进入，初始化参数
    {
        self->pri.peakType = 0;
        self->pri.peakCount = 0;
        self->pri.peakMaxCount = 0;
        self->pri.peak1 = 0;
        self->pri.peak2 = 0;
        self->pri.justchanged = FALSE;
        self->pri.setpoint = refVal; // 不变
        self->pri.running = TRUE;
        self->pri.outputStart = *self->pri.output;
        *self->pri.output = self->pri.outputStart + self->pri.oStep;
    }

    // 根据输入与设定点的关系振荡输出
    if (refVal > (self->pri.setpoint + self->pri.noiseBand))
        *self->pri.output = self->pri.outputStart - self->pri.oStep;
    else if (refVal < (self->pri.setpoint - self->pri.noiseBand))
        *self->pri.output = self->pri.outputStart + self->pri.oStep;

    // bool isMax=TRUE, isMin=TRUE;
    self->pri.isMax = TRUE;
    self->pri.isMin = TRUE;
    // id peaks
    /*
      以下循环完成，对回溯次数的输入缓存进行判断，如果输入值均大于或小于缓存值，则确定此次为峰值。
      峰值特征根据 isMax、isMin 哪个为真确定。
      同时完成输入缓存向后平移，腾出第一个单元存放新的输入值。
      这一段代码完成的噪声所产生的虚假峰值判断，应该没有问题！
    */
    for (i = self->pri.nLookBack - 1; i >= 0; i--)
    {
        int32_t val = self->pri.lastInputs[i];
        if (self->pri.isMax)
            self->pri.isMax = (refVal > val); // 第一次是新输入和缓存最后一个值比较，如果大于，则前面的值均判是否大于
        if (self->pri.isMin)
            self->pri.isMin = (refVal < val);                  // 第一次是新输入和缓存最后一个值比较，如果小于，则前面的值均判是否小于
        self->pri.lastInputs[i + 1] = self->pri.lastInputs[i]; // 每采样一次，将输入缓存的数据向后挪一次
    }
    self->pri.lastInputs[0] = refVal; // 新采样的数据放置缓存第一个单元。

    /*
    以下代码完成峰值的确定，以及对应峰值的时间纪录。
    因为上述代码只是去掉噪产生的波动峰值，但如果是连续超过 nLookBack 次数的的上升或下降，
    则上述算法所确定的最大或最小值，并非是峰值，只能是前 nLookBack 次中的最大或最小值。
    但逐句消化程序后，发现这段处理有几点疑惑：
    1、peaks[] 的纪录好像不对，在执行最小到最大值转换时，peakCount 也应该+1，否则应该把
       纪录的最小值覆盖了！所以后面的峰值判断总是满足条件。
    2、峰值对应时间似乎也应该多次存放，取平均值，因对象没有那么理想化，目前应该是取的最后一组峰值的周期。
    3、后续计算 Ku 用的是整个整定过程的最大、最小值，这对于非理想的对象而言也不是很合适。

    考虑做如下改进：
    1）修改峰值纪录，设计12个峰值保存单元，存满12个峰值（6大、6小）后再计算。
    2）纪录 6 组最大值的间隔时间，作为最终计算 Pu 的数据。
  */
    if (self->pri.isMax)
    {
        if (self->pri.peakType == 0)
            self->pri.peakType = 1; // 首次最大值，初始化

        if (self->pri.peakType == -1) // 如果前一次为最小值，则标识目前进入最大值判断
        {
            self->pri.peakType = 1;       // 开始最大值判断
            self->pri.peakCount++;        // 峰值计数   by shenghao.xu
            self->pri.justchanged = TRUE; // 标识峰值转换
            if (self->pri.peak2 != 0)     // 已经纪录一次最大峰值对应时间后，开始记录峰值周期  by shenghao.xu
            {
                self->pri.peakPeriod[self->pri.peakMaxCount] = (int32_t)(self->pri.peak1 - self->pri.peak2); // 最大峰值间隔时间（即峰值周期）
                self->pri.peakMaxCount++;                                                                    // 最大峰值计数
            }
            self->pri.peak2 = self->pri.peak1; // 刷新上次最大值对应时间
        }
        self->pri.peak1 = now;                         // 保存最大值对应时间 peak1
        self->pri.peaks[self->pri.peakCount] = refVal; // 保存最大值
    }                                                  // 此段代码可以保证得到的是真正的最大值，因为peakType不变，则会不断刷新最大值
    else if (self->pri.isMin)
    {
        if (self->pri.peakType == 0)
            self->pri.peakType = -1; // 首次最小值，初始化

        if (self->pri.peakType == 1) // 如果前一次是最大值判断，则转入最小值判断
        {
            self->pri.peakType = -1; // 开始最小值判断
            self->pri.peakCount++;   // 峰值计数
            self->pri.justchanged = TRUE;
        }

        if (self->pri.peakCount < 10)
            self->pri.peaks[self->pri.peakCount] = refVal; // 只要类型不变，就不断刷新最小值
    }

    /*  by shenghao.xu
    以下计算是作为判断采集数据是否合适的部分，如果 2 次峰值判断条件满足，就结束整定过程，感觉不甚合理。
    拟修改为：
    1）计满 12 次峰值后再计算(到第 13 次)。
    2）不再判断是否合理，因为对象如果特性好，自然已经稳定，如果不好，再长时间也无效果。
    3）将后面5次的数据作为素材，去掉第一组数据，因为考虑第一组时对象可能处于过渡过程。
    4）用后 10 点得到的 9 个峰值差平均值作为 Ku 计算值中的 A，取代原来的整个过程的最大、最小值差。
    5）用后 5 点峰值周期平均值作为 Pu 的计算值，取代原来用最后一组的值。
  */
    if (self->pri.justchanged && self->pri.peakCount == 12)
    {
        // we've transitioned.  check if we can autotune based on the last peaks
        iSum = 0;
        for (i = 2; i <= 10; i++)
            iSum += ABS(self->pri.peaks[i] - self->pri.peaks[i + 1]);
        iSum /= 9;                                                              // 取 9 次峰峰值平均值
        self->pri.Ku = (float32)(4 * (2 * self->pri.oStep)) / (iSum * 3.14159); // 用峰峰平均值计算 Ku

        iSum = 0;
        for (i = 1; i <= 5; i++)
            iSum += self->pri.peakPeriod[i];
        iSum /= 5;                             // 计算峰值的所有周期平均值
        self->pri.Pu = (float32)(iSum) / 1000; // 用周期平均值作为 Pu，单位：秒

        *self->pri.output = 0;
        self->pri.running = FALSE;
        return 1;
    }

    self->pri.justchanged = FALSE;
    return 0;
}

void pid_auto_tune_constructor(struct PID_AUTO_TUNE *self)
{
    self->set_ctrl_prm = _set_ctrl_prm;
    self->runtime = _runtime;
    self->set_output_step = _set_output_step;
    self->set_control_type = _set_control_type;
    self->set_noise_band = _set_noise_band;
    self->set_look_back = _set_look_back;

    self->get_kp = _get_kp;
    self->get_ki = _get_ki;
    self->get_kd = _get_kd;
}
