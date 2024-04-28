/**
 * @file data_analysis.h
 * @author xxx
 * @date 2023-06-25 13:07:02
 * @brief 处理传输层的数据
 * @copyright Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#ifndef COMPONENTS_COMMON_INCLUDE_DATA_ANALYSIS_H_
#define COMPONENTS_COMMON_INCLUDE_DATA_ANALYSIS_H_
#include "data_type_def.h"

typedef enum
{
    DATA_1,
    DATA_2,
    DATA_MAX,
} DataId_t; // 处理数据模块的个数，请根据实际情况修改

#define DATA_NUM (DATA_MAX)

#define DATA_BUF_RECV_SQQ_LEN 650u
#define DATA_BUF_SEND_SQQ_LEN 0u

#define DATA_SD_LEN_MAX 2
#define DATA_LD_LEN_MAX 2
#define DATA_ED_LEN_MAX 1

typedef struct _data_reg_t_
{
    struct
    {
        uint8_t len;
        uint8_t pos;
        uint8_t data[DATA_SD_LEN_MAX];
        BOOL valid; // 是否有效
    } sd;           // start delimiter

    struct
    {
        uint8_t len;
        uint8_t pos; // 偏移量，在wait_end_state中根据帧长去掉固定长度来判断是否是结束符
        uint8_t little_endian;
        BOOL valid; // 是否有效
    } ld;           // length describe

    struct
    {
        uint16_t len_max;
        uint16_t len_min;
    } argu;

    struct
    {
        uint8_t len;
        uint8_t data[DATA_ED_LEN_MAX];
        BOOL valid;
    } ed;

    BOOL echo_en;
    void (*func_ptr)(void);
} data_reg_t;

typedef void (*data_interupt_cb_t)(uint8_t id, uint8_t ch); ///< 中断回调函数，数据从这里写入

extern uint8_t data_read(uint8_t id, void *buffer, uint16_t len); ///< 读取数据

extern void data_write(uint8_t id, uint8_t *const string, uint16_t len); ///< TODO 写入数据

extern void lock_data(uint8_t data_id); ///< 锁定数据，防止中断写入数据

extern void unlock_data(uint8_t data_id); ///< 解锁数据

extern data_interupt_cb_t data_fsm_init(uint8_t data_id); ///< 初始化数据状态机

extern BOOL data_reg(uint8_t id, data_reg_t reg); ///< 注册数据

extern void data_unreg(uint8_t id); ///< 注销数据
#endif                              /* COMPONENTS_COMMON_INCLUDE_DATA_ANALYSIS_H_ */
