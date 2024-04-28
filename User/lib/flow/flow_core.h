/**
 * @file flow_core.h
 * @author: xxx
 * @date: 2023-07-21 17:00:15
 * @brief
 * @copyright: Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#ifndef __FLOW_CORE_
#define __FLOW_CORE_

#include "flow_def.h"

// 在定时器中断中调用
#define FLOW_TICK_UPDATE() \
    do                     \
    {                      \
        flow_tick++;       \
    } while (0);

// 初始化一个flow进程
#define FLOW_INIT(f) ((f)->line = 0)

// flow头，必须放在函数内的最前面
#define FLOW_HEAD(f)                      \
    {                                     \
        volatile char lock_once_flag = 0; \
        switch ((f)->line)                \
        {                                 \
        case 0:
// flow尾，必须放在函数内的最后面
#define FLOW_TAIL(f)                \
    }                               \
    ;                               \
    lock_once_flag = (f)->line = 0; \
    return FLOW_END;                \
    }                               \
    ;

// 给进程加锁，直到judge为真，加锁期间一直放开cpu给其他进程使用
#define FLOW_LOCK_WAIT(f, judge) \
    do                           \
    {                            \
        (f)->line = __LINE__;    \
    case __LINE__:;              \
        if (!(judge))            \
            return FLOW_WAIT;    \
    } while (0)

// 如果judge为真，就一直给进程加锁，加锁期间一直放开cpu给其他进程使用
#define FLOW_LOCK_WHILE(f, judge) \
    do                            \
    {                             \
        (f)->line = __LINE__;     \
    case __LINE__:;               \
        if (judge)                \
            return FLOW_WAIT;     \
    } while (0)

// 退出该进程
#define FLOW_EXIT(f)        \
    do                      \
    {                       \
        (f)->line = 0;      \
        return FLOW_FINISH; \
    } while (0)

// 无条件锁住进程一次，下次进来再接着往下运行
#define FLOW_LOCK_ONCE(f)     \
    do                        \
    {                         \
        lock_once_flag = 1;   \
        (f)->line = __LINE__; \
    case __LINE__:;           \
        if (lock_once_flag)   \
            return FLOW_LOCK; \
    } while (0)

// 等待一个flow进程结束
#define FLOW_WAIT_PROCESS_END(f, process) FLOW_LOCK_WHILE(f, (process) < FLOW_FINISH)

// 等待一个flow子进程结束
#define FLOW_WAIT_CHILD_PROCESS_END(f, cf, process) \
    do                                              \
    {                                               \
        FLOW_INIT((cf));                            \
        FLOW_WAIT_PROCESS_END((f), (process));      \
    } while (0)

// 给进程加锁，时长为time，加锁期间一直放开cpu给其他进程使用，time如果用FL_CLOCK_SEC来乘，那么time的单位就是s
#define FLOW_LOCK_DELAY(f, t)                                  \
    do                                                         \
    {                                                          \
        (f)->time = flow_tick;                                 \
        FLOW_LOCK_WAIT((f), ((flow_tick - (f)->time) >= (t))); \
    } while (0)

// 给进程加锁，时长为time，延时期间如果judge为真，就直接解锁进程
#define FLOW_LOCK_DELAY_OR_WAIT(f, judge, t)                                \
    do                                                                      \
    {                                                                       \
        (f)->time = flow_tick;                                              \
        FLOW_LOCK_WAIT((f), ((judge) || ((flow_tick - (f)->time) >= (t)))); \
    } while (0)

#endif /* __FLOW_CORE_ */
