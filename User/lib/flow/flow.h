/**
 * @file flow.h
 * @author: xxx
 * @date: 2023-07-21 17:00:15
 * @brief
 * @copyright: Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#ifndef __FLOW_
#define __FLOW_

#include "flow_def.h"
#include "flow_core.h"
#include "flow_sem.h"

#define FL_HARD_TICK (10U)                  /* 系统硬件中断一次所需要的时间，单位ms */
#define FL_CLOCK_SEC (1000U / FL_HARD_TICK) /* 一秒钟需要的tick，可以根据需求添加其他时间更短的宏 */
#define FL_CLOCK_100MSEC (100U / FL_HARD_TICK)
#define FL_CLOCK_10MSEC (FL_CLOCK_100MSEC / 10U)

/**
 * 初始化一个flow进程
 */
#define FL_INIT(fl) FLOW_INIT((fl))

/**
 * flow头，必须放在函数内的最前面
 */
#define FL_HEAD(fl) FLOW_HEAD((fl))

/**
 * flow尾，必须放在函数内的最后面
 */
#define FL_TAIL(fl) FLOW_TAIL((fl))

/**
 * 给进程加锁，直到judge为真，加锁期间一直放开cpu给其他进程使用
 */
#define FL_LOCK_WAIT(fl, judge) FLOW_LOCK_WAIT((fl), (judge))

/**
 * 如果judge为真，就一直给进程加锁，加锁期间一直放开cpu给其他进程使用
 */
#define FL_LOCK_WHILE(fl, judge) FLOW_LOCK_WHILE((fl), (judge))

/**
 * 退出该进程
 */
#define FL_EXIT(fl) FLOW_EXIT((fl))

/**
 * 无条件锁住进程一次，下次进来再接着往下运行
 */
#define FL_LOCK_ONCE(fl) FLOW_LOCK_ONCE((fl))

/**
 * 等待一个flow进程结束
 */
#define FL_WAIT_PROCESS_END(fl, process) FLOW_WAIT_PROCESS_END((fl), (process))

/**
 * 等待一个flow子进程结束
 */
#define FL_WAIT_CHILD(fl, cfl, process) FLOW_WAIT_CHILD_PROCESS_END((fl), (cfl), (process))

/**
 * 给进程加锁，时长为time，加锁期间一直放开cpu给其他进程使用，time如果用FL_CLOCK_SEC来乘，那么time的单位就是s
 * 此处time必须是常数
 */
#define FL_LOCK_DELAY(fl, time) FLOW_LOCK_DELAY((fl), (time))

/**
 * 给进程加锁，时长为time，延时期间如果judge为真，就直接解锁进程
 * 此处time必须是常数
 */
#define FL_LOCK_DELAY_OR_WAIT(fl, judge, time) FLOW_LOCK_DELAY_OR_WAIT((fl), (judge), (time))

/**
 * 初始化一个信号量
 */
#define FL_SEM_INIT(sem, count) FLOW_SEM_INIT((sem), (count))

/**
 * 给进程加锁，直到有信号释放
 */
#define FL_LOCK_WAIT_SEM(fl, sem) FLOW_LOCK_WAIT_SEM((fl), (sem))

/**
 * 给进程加锁，直到有信号或者超时，此处time可以为常数或者变量，其他的接口处time必须是常数
 */
#define FL_LOCK_WAIT_SEM_OR_TIMEOUT(fl, sem, time) FLOW_LOCK_WAIT_SEM_OR_TIMEOUT((fl), (sem), (time))

/**
 * 释放一个信号量
 */
#define FL_SEM_RELEASE(sem) FLOW_SEM_RELEASE((sem))

/**
 * 检测一个信号量是否被释放
 */
#define FL_SEM_IS_RELEASE(fl ,sem) FLOW_SEM_IS_RELEASE((fl), (sem))

/**
 * 初始化一个软件定时器
 */
void fl_timer_set(struct flow_timer *t, unsigned long interval);

/**
 * 复位一个软件定时器
 */
void fl_timer_reset(struct flow_timer *t);

/**
 * 重启一个软件定时器
 */
void fl_timer_restart(struct flow_timer *t);

/**
 * 检测一个软件定时器是否超时，0为不超时，1为超时
 */
unsigned char fl_timer_timeout(struct flow_timer *t);

/**
 * 检测一个软件定时器还剩多少时间超时，单位为硬件tick，比如硬件tick 500ms中断一次，那么
 * 返回的剩余时间就是500ms*n
 */
unsigned long fl_hour_much_time(struct flow_timer *t);

#endif /* __FLOW_ */
