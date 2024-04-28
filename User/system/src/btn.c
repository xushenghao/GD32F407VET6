/*
 * @Author:
 * @Date: 2023-07-04 08:25:56
 * @LastEditors: xxx
 * @LastEditTime: 2023-08-25 11:13:52
 * @Description:一个小巧简单易用的事件驱动型按键驱动模块，可无限量扩展按键，按键事件的回调异步处理方式可以简化你的程序结构，去除冗余的按键处理硬编码，让你的按键业务逻辑更清晰
 * email:
 * Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#include "../inc/btn.h"

#define EVENT_CB(ev)    \
	if (handle->cb[ev]) \
	handle->cb[ev]((Button *)handle)

// button handle list head.
static struct Button *head_handle = NULL;

/**
 * @brief  初始化按钮结构体句柄。
 * @param  handle: 按钮句柄结构体。
 * @param  pin_level: 读取按钮连接的HAL GPIOLevel。
 * @param  active_level: 按下按钮的GPIOLevel。
 * @param  button_id: 按钮ID。
 * @retval 无
 */
void button_init(struct Button *handle, uint8_t (*pin_level)(uint8_t), active_level_e active_level, uint8_t button_id)
{
#ifdef STM32
	osel_memset((uint8_t *)handle, 0, sizeof(struct Button));
#else
	memset(handle, 0, sizeof(struct Button));
#endif

	handle->event = (uint8_t)NONE_PRESS;
	handle->hal_button_Level = pin_level;
	handle->button_level = handle->hal_button_Level(button_id);
	handle->active_level = (uint8_t)active_level;
	handle->button_id = button_id;
}

/**
 * @brief  为按钮添加事件回调函数。
 * @param  handle: 按钮句柄结构体。
 * @param  event: 触发事件类型。
 * @param  cb: 回调函数。
 * @retval 无
 */
void button_attach(struct Button *handle, PressEvent event, BtnCallback cb)
{
	handle->cb[event] = cb;
}

/**
 * @brief  查询按钮发生的事件。
 * @param  handle: 按钮句柄结构体。
 * @retval 按钮事件。
 */
PressEvent get_button_event(struct Button *handle)
{
	return (PressEvent)(handle->event);
}

/**
 * @brief  按钮驱动核心函数，驱动状态机。
 * @param  handle: 按钮句柄结构体。
 * @retval 无
 */
void button_handler(struct Button *handle)
{
	uint8_t read_gpio_level = handle->hal_button_Level(handle->button_id);

	// ticks counter working..
	if ((handle->state) > 0)
		handle->ticks++;

	/*------------button debounce handle---------------*/
	if (read_gpio_level != handle->button_level)
	{ // not equal to prev one
		// continue read 3 times same new level change
		if (++(handle->debounce_cnt) >= DEBOUNCE_TICKS)
		{
			handle->button_level = read_gpio_level;
			handle->debounce_cnt = 0;
		}
	}
	else
	{ // leved not change ,counter reset.
		handle->debounce_cnt = 0;
	}

	/*-----------------State machine-------------------*/
	switch (handle->state)
	{
	case 0:
		if (handle->button_level == handle->active_level)
		{ // start press down
			handle->event = (uint8_t)PRESS_DOWN;
			EVENT_CB(PRESS_DOWN);
			handle->ticks = 0;
			handle->repeat = 1;
			handle->state = 1;
		}
		else
		{
			handle->event = (uint8_t)NONE_PRESS;
		}
		break;

	case 1:
		if (handle->button_level != handle->active_level)
		{ // released press up
			handle->event = (uint8_t)PRESS_UP;
			EVENT_CB(PRESS_UP);
			handle->ticks = 0;
			handle->state = 2;
		}
		else if (handle->ticks > LONG_TICKS)
		{
			handle->event = (uint8_t)LONG_PRESS_START;
			EVENT_CB(LONG_PRESS_START);
			handle->state = 5;
		}
		break;

	case 2:
		if (handle->button_level == handle->active_level)
		{ // press down again
			handle->event = (uint8_t)PRESS_DOWN;
			EVENT_CB(PRESS_DOWN);
			handle->repeat++;
			EVENT_CB(PRESS_REPEAT); // repeat hit
			handle->ticks = 0;
			handle->state = 3;
		}
		else if (handle->ticks > SHORT_TICKS)
		{ // released timeout
			if (handle->repeat == 1)
			{
				handle->event = (uint8_t)SINGLE_CLICK;
				EVENT_CB(SINGLE_CLICK);
			}
			else if (handle->repeat == 2)
			{
				handle->event = (uint8_t)DOUBLE_CLICK;
				EVENT_CB(DOUBLE_CLICK); // repeat hit
			}
			handle->state = 0;
		}
		break;

	case 3:
		if (handle->button_level != handle->active_level)
		{ // released press up
			handle->event = (uint8_t)PRESS_UP;
			EVENT_CB(PRESS_UP);
			if (handle->ticks < SHORT_TICKS)
			{
				handle->ticks = 0;
				handle->state = 2; // repeat press
			}
			else
			{
				handle->state = 0;
			}
		}
		else if (handle->ticks > SHORT_TICKS)
		{ // long press up
			handle->state = 0;
		}
		break;

	case 5:
		if (handle->button_level == handle->active_level)
		{
			// continue hold trigger
			handle->event = (uint8_t)LONG_PRESS_HOLD;
			EVENT_CB(LONG_PRESS_HOLD);
		}
		else
		{ // releasd
			handle->event = (uint8_t)PRESS_UP;
			EVENT_CB(PRESS_UP);
			handle->state = 0; // reset
		}
		break;
	default:
		handle->state = 0; // reset
		break;
	}
}

/**
 * @brief  启动按钮工作，将句柄添加到工作队列中。
 * @param  handle: 目标句柄结构体。
 * @retval 0: 成功。-1: 已存在。
 */
int button_start(struct Button *handle)
{
	struct Button *target = head_handle;
	while (target)
	{
		if (target == handle)
			return -1; // already exist.
		target = target->next;
	}
	handle->next = head_handle;
	head_handle = handle;
	return 0;
}

/**
 * @brief  停止按钮工作，从工作队列中移除句柄。
 * @param  handle: 目标句柄结构体。
 * @retval None
 */
void button_stop(struct Button *handle)
{
	struct Button **curr;
	for (curr = &head_handle; *curr;)
	{
		struct Button *entry = *curr;
		if (entry == handle)
		{
			*curr = entry->next;
			//			free(entry);
			return;
		}
		else
			curr = &entry->next;
	}
}

/**
 * @brief  后台计时，定时器重复调用间隔为5ms。
 * @param  None.
 * @retval None
 */
void button_ticks()
{
	struct Button *target;
	for (target = head_handle; target; target = target->next)
	{
		button_handler(target);
	}
}
