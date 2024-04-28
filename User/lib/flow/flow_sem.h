/***
 * @file:
 * @author: xxx
 * @date: 2023-07-21 17:00:15
 * @brief
 * @copyright: Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#ifndef __FLOW_SEM_H__
#define __FLOW_SEM_H__

#include "flow_def.h"
#include "flow_core.h"

#define FLOW_SEM_INIT(s, c) ((s)->count = c) // 初始化信号量s的计数值为c

// 等待信号量s的计数值大于0
#define FLOW_LOCK_WAIT_SEM(f, s)           \
    do                                     \
    {                                      \
        FLOW_LOCK_WAIT(f, (s)->count > 0); \
        --(s)->count;                      \
    } while (0)

// 等待信号量s的计数值大于0，或者当前时间与锁f的时间之差大于等于t
#define FLOW_LOCK_WAIT_SEM_OR_TIMEOUT(f, s, t)                                             \
    do                                                                                     \
    {                                                                                      \
        (f)->time = flow_tick;                                                             \
        (s)->time = (t);                                                                   \
        FLOW_LOCK_WAIT(f, (((s)->count > 0) || ((flow_tick - (f)->time) >= ((s)->time)))); \
        if (((s)->count > 0) && ((flow_tick - (f)->time) < ((s)->time)))                   \
            --(s)->count;                                                                  \
    } while (0)

#define FLOW_SEM_RELEASE(s) (++(s)->count)

#define FLOW_SEM_IS_RELEASE(f, s) (flow_tick - (f)->time) < ((s)->time)

#endif /* __FLOW_SEM_H__ */
