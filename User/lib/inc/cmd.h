/**
 * @file cmd.h
 * @author xxx
 * @date 2023-06-25 13:07:02
 * @brief 命令解析器
 * @copyright Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#ifndef _CMD_H_
#define _CMD_H_

#define CMD_HASH 0xb433e5c6

#if defined(__CC_ARM) || defined(__CLANG_ARM) /* ARM Compiler */
#define SECTION(x) __attribute__((section(x)))
#define CMD_USED __attribute__((used))

#elif defined(__IAR_SYSTEMS_ICC__) /* IAR Compiler */
#define SECTION(x) @x
#define CMD_USED __root
#else
#error "not supported tool chain..."
#endif

typedef void (*cmd_handler)(void);

typedef struct cmd
{
    const char *cmd;
    const char *cmd_mess;
    unsigned int hash;
    cmd_handler handler;
} cmd_t;

/// 注册命令
#define REGISTER_CMD(cmd, handler, desc)             \
    const char _register_##cmd##_cmd[] = #cmd;       \
    const char _register_##cmd##_desc[] = #desc;     \
    CMD_USED cmd_t _register_##cmd SECTION("CMDS") = \
        {                                            \
            _register_##cmd##_cmd,                   \
            _register_##cmd##_desc,                  \
            (unsigned int)CMD_HASH,                  \
            (cmd_handler)&handler};

void cmd_init(void);         ///< 初始化命令
void cmd_parsing(char *str); ///< 命令解析

#endif
