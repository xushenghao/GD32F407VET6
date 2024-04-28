/***
 * @Author:
 * @Date: 2023-03-20 19:27:47
 * @LastEditors: xxx
 * @LastEditTime: 2023-03-30 00:34:41
 * @Description:日志打印模块PC端调试使用
 * @email:
 * @Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#ifndef __LOG_H_
#define __LOG_H_
#include <string.h>
#include <stdio.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)

/*调试日志宏定义*/

#define LOG_PRINT(fmt, ...)                                                                         \
    do                                                                                              \
    {                                                                                               \
        printf("[DEBUG:%s][%s:%d] " fmt "\n", __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0);

/*错误日志打印(在日志打印模块还未启动时使用)*/
#define LOG_ERR(fmt, ...)                                                                           \
    do                                                                                              \
    {                                                                                               \
        printf("[ERROR:%s][%s:%d] " fmt "\n", __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0);

// 打印十六进制字符串
#define LOG_HEX(data, len)                                                  \
    do                                                                      \
    {                                                                       \
        printf("[DEBUG:%s][%s:%d] ", __FILENAME__, __FUNCTION__, __LINE__); \
        for (int i = 0; i < len; i++)                                       \
        {                                                                   \
            printf("%02x ", data[i]);                                       \
        }                                                                   \
        printf("\n");                                                       \
    } while (0);

#endif //__LOG_H_
