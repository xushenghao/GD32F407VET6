/**
 * @file
 * @author xxx
 * @date 2023-07-21 17:00:15
 * @brief
 * @copyright Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#include "flow.h"

unsigned long flow_tick;

/**
 * @brief 设置流量计器的间隔时间。
 * @param t 要设置的流量计器。
 * @param interval 要设置的间隔时间值。
 * @return 如果间隔时间无效或无法设置，则返回NULL。
 * @note 此函数设置间隔时间并重置计时器，因此它将在下一个间隔时间 occurs时开始。
 */
void fl_timer_set(struct flow_timer *t, unsigned long interval)
{
    if (interval == 0)
    {
        // 如果间隔时间为零，返回错误
        return;
    }

    t->interval = interval;
    t->start = flow_tick;
}

/**
 * @brief 重置流量计器。
 * @param t 要重置的流量计器。
 * @return 如果无法重置计时器，则返回NULL。
 * @note 此函数将计时器的开始时间加上间隔时间，因此它将在下一个间隔时间 occurs时开始。
 */
void fl_timer_reset(struct flow_timer *t)
{
    t->start += t->interval;
}

/**
 * @brief 重新启动流量计器。
 * @param t 要重新启动的流量计器。
 * @return 如果无法重新启动计时器，则返回NULL。
 * @note 此函数将计时器的开始时间设置为当前flow_tick，因此它将在重新开始时从基本开始。
 */
void fl_timer_restart(struct flow_timer *t)
{
    t->start = flow_tick;
}

/**
 * @brief 检查给定的flow_timer结构体的超时状态
 * @param {flow_timer} *t 指向flow_timer结构体的指针
 * @return {unsigned char} 超时返回1，否则返回0
 * @note 检查当前时间与flow_timer结构体中的start时间之差是否大于或等于interval
 */
unsigned char fl_timer_timeout(struct flow_timer *t)
{
    return ((flow_tick - t->start) >= t->interval) ? 1U : 0U;
}

/**
 * @brief 计算给定flow_timer结构体中的时间长度
 * @param {flow_timer} *t 指向flow_timer结构体的指针
 * @return {unsigned long} 返回时间长度
 * @note 计算start时间加上interval与当前时间flow_tick之差，如果大于等于flow_tick，则返回time_len - flow_tick，否则返回0
 */
unsigned long fl_hour_much_time(struct flow_timer *t)
{
    unsigned long time_len = t->start + t->interval;

    if (time_len >= flow_tick)
    {
        return (time_len - flow_tick);
    }
    else
    {
        return 0U;
    }
}
