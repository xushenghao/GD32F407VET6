/**
 * @file flow_def.h
 * @author: xxx
 * @date: 2023-07-21 17:00:15
 * @brief
 * @copyright: Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#ifndef __FLOW_DEF_
#define __FLOW_DEF_

#define FLOW_WAIT (0)
#define FLOW_LOCK (1)
#define FLOW_FINISH (2)
#define FLOW_END (3)

struct flow
{
    unsigned long line;
    unsigned long time;
};

struct flow_timer
{
    unsigned long start;
    unsigned long interval;
};

struct flow_sem
{
    unsigned long count;
    unsigned long time;
};

extern unsigned long flow_tick;

#endif /* __FLOW_DEF_ */
