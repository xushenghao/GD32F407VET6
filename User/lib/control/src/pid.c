#include "pid.h"
#include <math.h>

// 构造函数将接口绑定
void pid_constructor(pid_t *self)
{
    switch (self->type)
    {
    case PID_TYPE_COMMON:
        /* code */
        break;
    case PID_TYPE_NEURAL:
        pid_neural_constructor(&self->pid_u.neural);
        break;
    case PID_TYPE_FUZZY:
        pid_fuzzy_constructor(&self->pid_u.fuzzy);
        break;
    case PID_TYPE_AUTO_TUNE:
        pid_auto_tune_constructor(&self->auto_tune);
        break;
    case PID_TYPE_CUSTOM_CAO:
        pid_c_constructor(&self->pid_u.cao);
        break;
    case PID_TYPE_CUSTOM_GAO:
        pid_g_constructor(&self->pid_u.gao);
        break;
    case PID_TYPE_CUSTOM_XU:
        pid_x_constructor(&self->pid_u.xu);
        break;
    case PID_TYPE_CUSTOM_ZHANG:
        pid_zh_constructor(&self->pid_u.zhang);
        break;
    case PID_TYPE_CUSTOM_HANGDIAN:
        pid_hd_constructor(&self->pid_u.hd);
        break;
    default:
        break;
    }
}
