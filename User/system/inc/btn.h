/***
 * @Author:
 * @Date: 2023-07-04 08:26:12
 * @LastEditors: xxx
 * @LastEditTime: 2023-08-09 22:49:14
 * @Description: btn是一个小巧简单易用的事件驱动型按键驱动模块，可无限量扩展按键，按键事件的回调异步处理方式
 * @email:
 * @Copyright (c) 2023 by xxx, All Rights Reserved.
 */

/**
使用方法：
1.先申请一个按键结构

struct Button button1;
2.初始化按键对象，绑定按键的GPIO电平读取接口read_button_pin() ，后一个参数设置有效触发电平

button_init(&button1, read_button_pin, 0, 0);
3.注册按键事件

button_attach(&button1, SINGLE_CLICK, Callback_SINGLE_CLICK_Handler);
button_attach(&button1, DOUBLE_CLICK, Callback_DOUBLE_Click_Handler);
...
4.启动按键

button_start(&button1);
5.设置一个5ms间隔的定时器循环调用后台处理函数

while(1) {
	...
	if(timer_ticks == 5) {
		timer_ticks = 0;

		button_ticks();
	}
}
*/
#ifndef _BTN_H_
#define _BTN_H_

#ifdef STM32
#include "lib.h"
#else
#include "stdint.h"
#include "string.h"
#endif

// 根据您的需求修改常量。
#define TICKS_INTERVAL 10				   // 按钮扫描间隔，单位ms
#define DEBOUNCE_TICKS 3				   // 按键去抖动时间，单位ms
#define SHORT_TICKS (100 / TICKS_INTERVAL) // 短按时间阈值，单位ms
#define LONG_TICKS (500 / TICKS_INTERVAL)  // 长按时间阈值，单位ms

typedef void (*BtnCallback)(void *);

typedef enum
{
	ACTIVE_LEVEL_LOW = 0, // 低电平有效
	ACTIVE_LEVEL_HIGH,	  // 高电平有效
} active_level_e;

/***
 * @brief
	事件	说明
	PRESS_DOWN	按键按下，每次按下都触发
	PRESS_UP	按键弹起，每次松开都触发
	PRESS_REPEAT	重复按下触发，变量repeat计数连击次数
	SINGLE_CLICK	单击按键事件
	DOUBLE_CLICK	双击按键事件
	LONG_PRESS_START	达到长按时间阈值时触发一次
	LONG_PRESS_HOLD	长按期间一直触发
 */
typedef enum
{
	PRESS_DOWN = 0,	  // 按下
	PRESS_UP,		  // 弹起
	PRESS_REPEAT,	  // 重复按下
	SINGLE_CLICK,	  // 单击
	DOUBLE_CLICK,	  // 双击
	LONG_PRESS_START, // 长按开始
	LONG_PRESS_HOLD,  // 长按保持
	number_of_event,  // 事件数量
	NONE_PRESS		  // 无按键
} PressEvent;

/***
 * @brief 按钮设备结构体
 */
typedef struct Button
{
	uint16_t ticks;									 // 计时器
	uint8_t repeat : 4;								 // 重复计数
	uint8_t event : 4;								 // 事件类型
	uint8_t state : 3;								 // 状态
	uint8_t debounce_cnt : 3;						 // 去抖计数
	uint8_t active_level : 1;						 // 激活电平
	uint8_t button_level : 1;						 // 按钮电平
	uint8_t button_id;								 // 按钮ID
	uint8_t (*hal_button_Level)(uint8_t button_id_); // 获取按钮引脚电平函数
	BtnCallback cb[number_of_event];				 // 回调函数数组
	struct Button *next;							 // 下一个按钮句柄
} Button;

#ifdef __cplusplus
extern "C"
{
#endif

	/**
	 * @brief 初始化按钮设备
	 * @param handle: 按钮句柄结构体
	 * @param pin_level: 获取按钮引脚电平函数
	 * @param active_level: 按钮激活电平
	 * @param button_id: 按钮ID
	 * @retval 0: 成功
	 * @retval -1: 失败
	 */
	void button_init(struct Button *handle, uint8_t (*pin_level)(uint8_t), active_level_e active_level, uint8_t button_id);

	/**
	 * @brief 附加按钮事件处理函数
	 * @param handle: 按钮句柄结构体
	 * @param event: 按钮事件
	 * @param cb: 回调函数
	 * @retval 0: 成功
	 * @retval -1: 失败
	 */
	void button_attach(struct Button *handle, PressEvent event, BtnCallback cb);

	/**
	 * @brief 获取按钮事件
	 * @param handle: 按钮句柄结构体
	 * @retval 按钮事件
	 */
	PressEvent get_button_event(struct Button *handle);

	/**
	 * @brief 启动按钮工作
	 * @param handle: 按钮句柄结构体
	 * @retval 0: 成功
	 * @retval -1: 已存在
	 */
	int button_start(struct Button *handle);

	/**
	 * @brief 停止按钮工作
	 * @param handle: 按钮句柄结构体
	 * @retval None
	 */
	void button_stop(struct Button *handle);

	/**
	 * @brief 后台计时，定时器重复调用间隔为5ms
	 * @param None
	 * @retval None
	 */
	void button_ticks(void);

#ifdef __cplusplus
}
#endif

#endif
