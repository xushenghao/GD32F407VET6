/**
 * @file data_analysis.c
 * @author xxx
 * @date 2023-06-25 13:07:02
 * @brief 处理传输层的数据
 * @copyright Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#include <string.h>
#include "../inc/data_analysis.h"
#include "../inc/sqqueue.h"
#include "../inc/debug.h"

typedef uint8_t data_entry_t;
typedef void (*state_t)(uint8_t data_id, uint8_t sig, uint8_t ch);

typedef enum _event /* enumeration */
{
    SD_SIG,
    LD_SIG,
    CHAR_SIG,
    ED_SIG,
} sig_event;

typedef struct _fsm_t_
{
    state_t current_state;
} fsm_t;

typedef struct _DATA_frm_t_
{
    uint8_t sd_index;

    struct
    {
        uint8_t data[DATA_LD_LEN_MAX];
        uint8_t index;
        uint16_t frm_len;
    } ld; // length describe

    uint16_t payload_len; // actually len of recvived data
    uint8_t ed_index;

    uint16_t last_enter_q_num; // record the num of frames has entered the queue
    uint8_t locked;
    uint8_t sig;
} data_frm_t;

#define TRAN(state) (fsm[data_id].current_state = (state_t)(state))
#define FSM_DISPATCH(data_id, sig, ch) (fsm[data_id].current_state((data_id), (sig), (ch)))

static fsm_t fsm[DATA_NUM] = {0};

static data_frm_t data_frm_array[DATA_NUM];
static data_reg_t data_reg_array[DATA_NUM];

static sqqueue_ctrl_t data_recv_sqq[DATA_NUM];

static void wait_sd_state(uint8_t data_id, uint8_t sig, uint8_t ch);
/**
 * @brief 加锁数据
 * @param {uint8_t} data_id
 * @note
 */
void lock_data(uint8_t data_id)
{
    data_frm_array[data_id].locked = TRUE;                                 // 设置数据帧结构体的锁定标志为TRUE
    for (uint8_t i = 0; i < data_frm_array[data_id].last_enter_q_num; i++) // 遍历数据接收缓冲区，删除所有已入队项
    {
        data_recv_sqq[data_id].revoke(&data_recv_sqq[data_id]); // 删除项
    }

    data_frm_array[data_id].last_enter_q_num = 0; // 重置最后进入队列的项数量
    data_frm_array[data_id].sd_index = 0;         // 重置SD索引
    data_frm_array[data_id].ed_index = 0;         // 重置ED索引
    data_frm_array[data_id].ld.frm_len = 0;       // 重置数据帧长度
    data_frm_array[data_id].payload_len = 0;      // 重置负载长度

    TRAN(wait_sd_state); // 切换到等待SD状态
}

/**
 * @brief 解锁数据
 * @param {uint8_t} data_id
 * @note
 */
void unlock_data(uint8_t data_id)
{
    TRAN(wait_sd_state); // 切换到等待SD状态

    data_frm_array[data_id].last_enter_q_num = 0; // 重置最后进入队列的项数量
    data_frm_array[data_id].sd_index = 0;         // 重置SD索引
    data_frm_array[data_id].ed_index = 0;         // 重置ED索引
    data_frm_array[data_id].ld.frm_len = 0;       // 重置数据帧长度
    data_frm_array[data_id].payload_len = 0;      // 重置负载长度

    data_frm_array[data_id].locked = FALSE; // 设置数据帧结构体的锁定标志为FALSE
}

/**
 * @brief 处理结束状态
 * @param {uint8_t} data_id
 * @param {uint8_t} sig
 * @param {uint8_t} ch
 * @note
 */
static void end_state_handle(uint8_t data_id, uint8_t sig, uint8_t ch)
{
    TRAN(wait_sd_state);                          // 切换到等待SD状态
    data_frm_array[data_id].ld.frm_len = 0;       // 重置数据帧长度
    data_frm_array[data_id].payload_len = 0;      // 重置负载长度
    data_frm_array[data_id].last_enter_q_num = 0; // 重置最后进入队列的项数量
    if (data_reg_array[data_id].func_ptr != NULL) // 如果数据项注册了处理函数
    {
        (*(data_reg_array[data_id].func_ptr))(); // 调用处理函数
    }
}

/**
 * @brief 这个函数处理等待结束状态。
 * @param {uint8_t} data_id - 数据的ID。
 * @param {uint8_t} sig - 信号。
 * @param {uint8_t} ch - 通道。
 * @return {*}
 * @note
 */
static void wait_end_state(uint8_t data_id, uint8_t sig, uint8_t ch)
{
    // 如果数据寄存器数组无效
    if (!data_reg_array[data_id].ed.valid)
    {
        // 如果数据帧数组的帧长度为0
        if (data_frm_array[data_id].ld.frm_len == 0)
        {
            // 转换到等待SD状态
            TRAN(wait_sd_state);

            // 处理结束状态
            end_state_handle(data_id, sig, ch);

            // 对于数据帧数组的最后一个进入队列的数量，撤销数据接收队列
            for (uint8_t i = 0; i < data_frm_array[data_id].last_enter_q_num; i++)
            {
                data_recv_sqq[data_id].revoke(&data_recv_sqq[data_id]);
            }
            // 将数据帧数组的最后一个进入队列的数量设置为0
            data_frm_array[data_id].last_enter_q_num = 0;
        }
        else
        {
            // 如果数据接收队列进入成功
            if (data_recv_sqq[data_id].enter(&data_recv_sqq[data_id], (void *)&ch))
            {
                // 增加数据帧数组的最后一个进入队列的数量
                data_frm_array[data_id].last_enter_q_num++;
                // 如果增加的数据帧数组的有效载荷长度等于数据帧数组的帧长度
                if (++data_frm_array[data_id].payload_len ==
                    data_frm_array[data_id].ld.frm_len)
                {
                    // 处理结束状态
                    end_state_handle(data_id, sig, ch);
                }
            }
            else
            {
                // 锁定数据
                lock_data(data_id);
            }
        }
    }
    else
    {
        // 如果数据帧数组的帧长度为0
        if (data_frm_array[data_id].ld.frm_len == 0)
        {
            // 如果数据接收队列进入成功
            if (data_recv_sqq[data_id].enter(&data_recv_sqq[data_id], (void *)&ch))
            {
                // 增加数据帧数组的最后一个进入队列的数量
                data_frm_array[data_id].last_enter_q_num++;
                // 如果数据寄存器数组的数据等于通道
                if (data_reg_array[data_id].ed.data[0] == ch)
                {
                    // 处理结束状态
                    end_state_handle(data_id, sig, ch);
                }
            }
            else
            {
                // 锁定数据
                lock_data(data_id);
            }
        }
        else
        {
            // 如果数据接收队列进入成功
            if (data_recv_sqq[data_id].enter(&data_recv_sqq[data_id], (void *)&ch))
            {
                // 增加数据帧数组的最后一个进入队列的数量
                data_frm_array[data_id].last_enter_q_num++;
                // 如果增加的数据帧数组的有效载荷长度大于等于数据帧数组的帧长度减去数据寄存器数组的位置
                if (++data_frm_array[data_id].payload_len >=
                    data_frm_array[data_id].ld.frm_len - data_reg_array[data_id].ld.pos)
                {
                    // 如果数据寄存器数组的数据等于通道
                    if (data_reg_array[data_id].ed.data[0] == ch)
                    {
                        // 处理结束状态
                        end_state_handle(data_id, sig, ch);
                    }
                }
            }
            else
            {
                // 锁定数据
                lock_data(data_id);
            }
        }
    }
}

/**
 * @brief 处理等待LD状态
 * @param {uint8_t} data_id
 * @param {uint8_t} sig
 * @param {uint8_t} ch
 * @return {*}
 * @note
 */
static void wait_ld_state(uint8_t data_id, uint8_t sig, uint8_t ch)
{
    if (!data_reg_array[data_id].ld.valid) // 如果数据项未注册LD状态
    {
        TRAN(wait_end_state);                                   // 切换到等待结束状态
        FSM_DISPATCH(data_id, data_frm_array[data_id].sig, ch); // 调用FSM处理函数
        return;
    }
    data_frm_array[data_id].ld.data[data_frm_array[data_id].ld.index++] = ch; // 将字符添加到数据帧中
    if (data_recv_sqq[data_id].enter(&data_recv_sqq[data_id], (void *)&ch))   // 尝试进入队列
    {
        data_frm_array[data_id].last_enter_q_num++;                             // 增加最后进入队列的项数量
        if (data_frm_array[data_id].ld.index == data_reg_array[data_id].ld.len) // 如果索引等于数据项长度
        {
            if (data_reg_array[data_id].ld.little_endian == TRUE) // 如果小端存储
            {
                data_frm_array[data_id].ld.frm_len =
                    data_frm_array[data_id].ld.data[DATA_LD_LEN_MAX - 1] * 256 +
                    data_frm_array[data_id].ld.data[DATA_LD_LEN_MAX - 2];
            }
            else
            {
                data_frm_array[data_id].ld.frm_len =
                    data_frm_array[data_id].ld.data[DATA_LD_LEN_MAX - 2] * 256 +
                    data_frm_array[data_id].ld.data[DATA_LD_LEN_MAX - 1];
            }

            if (data_reg_array[data_id].ld.len == 1) // 如果是只有1个字节长度的数据
            {
                data_frm_array[data_id].ld.frm_len = data_frm_array[data_id].ld.data[0];
            }

            if ((data_frm_array[data_id].ld.frm_len > data_reg_array[data_id].argu.len_max) || (data_frm_array[data_id].ld.frm_len < data_reg_array[data_id].argu.len_min))
            {
                data_frm_array[data_id].ld.index = 0;
                TRAN(wait_sd_state); // 切换到等待SD状态

                for (uint8_t i = 0; i < data_frm_array[data_id].last_enter_q_num; i++)
                {
                    data_recv_sqq[data_id].revoke(&data_recv_sqq[data_id]); // 删除项
                }

                data_frm_array[data_id].ld.frm_len = 0;
                data_frm_array[data_id].last_enter_q_num = 0;
            }
            else
            {
                data_frm_array[data_id].ld.index = 0;
                TRAN(wait_end_state); // 切换到等待结束状态
            }
        }
    }
    else
    {
        lock_data(data_id); // 锁定数据
    }
}
/**
 * @brief 等待SD状态处理函数
 * @param {uint8_t} data_id 数据ID
 * @param {uint8_t} sig 信号
 * @param {uint8_t} ch  字符
 * @return {*}
 * @note
 */
static void wait_sd_state(uint8_t data_id, uint8_t sig, uint8_t ch)
{
    // 如果数据寄存器中的SD数据无效
    if (!data_reg_array[data_id].sd.valid)
    {
        //  transition to wait_ld_state状态
        TRAN(wait_ld_state);
        // 调用数据帧数组中对应的数据帧处理函数
        FSM_DISPATCH(data_id, data_frm_array[data_id].sig, ch);
        return;
    }
    // 如果数据寄存器中的SD数据中的当前字节等于输入的字节
    if (data_reg_array[data_id].sd.data[data_frm_array[data_id].sd_index++] == ch)
    {
        // 如果输入字符串成功进入队列
        if (data_recv_sqq[data_id].enter(&data_recv_sqq[data_id], (void *)&ch))
        {
            // 更新数据帧数组中对应的数据帧的最后一个进入队列的队列号
            data_frm_array[data_id].last_enter_q_num++;
            // 如果数据帧中的SD数据索引等于数据寄存器中的SD数据长度
            if (data_frm_array[data_id].sd_index == data_reg_array[data_id].sd.len)
            {
                // 更新数据帧中的SD数据索引为0
                data_frm_array[data_id].sd_index = 0;
                //  transition to wait_ld_state状态
                TRAN(wait_ld_state);
            }
        }
        // 如果输入字符串无法进入队列
        else
        {
            // 锁定数据
            lock_data(data_id);
        }
    }
    // 如果数据寄存器中的SD数据中的当前字节不等于输入的字节
    else
    {
        // 遍历并删除队列中的输入字符串
        for (uint8_t i = 0; i < data_frm_array[data_id].last_enter_q_num; i++)
        {
            data_recv_sqq[data_id].revoke(&data_recv_sqq[data_id]);
        }

        // 更新数据帧中的SD数据索引为0
        data_frm_array[data_id].sd_index = 0;
        // 更新数据帧中的最后一个进入队列的队列号为0
        data_frm_array[data_id].last_enter_q_num = 0;
    }
}

/**
 * @brief 处理数据字符
 * @param {uint8_t} data_id
 * @param {uint8_t} ch
 * @return {*}
 * @note
 */
static void data_char_handle(uint8_t data_id, uint8_t ch)
{
    // 如果数据ID对应的数据寄存器的回显使能标志为真
    if (data_reg_array[data_id].echo_en)
    {
        // 将输入字符写入数据寄存器
        data_write(data_id, &ch, 1);
    }

    // 调用数据帧数组中对应的数据帧处理函数
    FSM_DISPATCH(data_id, data_frm_array[data_id].sig, ch);
}

/**
 * @brief 初始化数据帧处理机
 * @param {uint8_t} data_id
 * @return {*}
 * @note
 */
data_interupt_cb_t data_fsm_init(uint8_t data_id)
{
    TRAN(wait_sd_state);                                     // 切换到等待SD状态
    data_reg_array[data_id].func_ptr = NULL;                 // 设置数据ID寄存器的回调函数为空
    memset(&data_frm_array[data_id], 0, sizeof(data_frm_t)); // 初始化数据帧结构体
    data_frm_array[data_id].sig = CHAR_SIG;                  // 设置数据帧签名

    if (sqqueue_ctrl_init(&data_recv_sqq[data_id],
                          sizeof(data_entry_t),
                          DATA_BUF_RECV_SQQ_LEN) == FALSE) // 初始化数据接收缓冲区
    {
        DBG_ASSERT(FALSE __DBG_LINE); // 如果初始化失败，输出调试信息
    }

    return data_char_handle; // 返回数据处理回调函数指针
}
/**
 * @brief 读取数据
 * @param {uint8_t} id
 * @param {void} *buffer
 * @param {uint16_t} len
 * @return {*}
 * @note
 */
uint8_t data_read(uint8_t id, void *buffer, uint16_t len)
{
    uint8_t i = 0;
    data_entry_t e;
    uint8_t *buf = (uint8_t *)buffer;

    // 如果接收队列中存在长度大于等于输入长度的数据项
    if (data_recv_sqq[id].get_len(&data_recv_sqq[id]) >= len)
    {
        // 遍历接收队列并读取数据项到缓冲区
        for (i = 0; i < len; i++)
        {
            e = *((data_entry_t *)data_recv_sqq[id].del(&data_recv_sqq[id]));
            buf[i] = e;
        }
    }
    // 如果接收队列中不存在长度大于等于输入长度的数据项
    else
    {
        // 遍历接收队列并读取数据项到缓冲区
        while ((data_recv_sqq[id].get_len(&data_recv_sqq[id]) != 0) && (i < len))
        {
            e = *((data_entry_t *)data_recv_sqq[id].del(&data_recv_sqq[id]));
            buf[i++] = e;
        }
    }

    // 如果数据帧数组中对应的数据帧被锁定
    if (data_frm_array[id].locked)
    {
        // 解锁数据
        unlock_data(id);
    }

    return i;
}

/**
 * @brief
 * @param {uint8_t} id
 * @param {uint8_t} *string
 * @param {uint16_t} len
 * @return {*}
 * @note
 */
void data_write(uint8_t id, uint8_t *const string, uint16_t len)
{
}

/**
 * @brief 设置数据寄存器
 * @param {uint8_t} id
 * @param {data_reg_t} reg
 * @return {*}
 * @note
 */
BOOL data_reg(uint8_t id, data_reg_t reg)
{
    if (data_reg_array[id].func_ptr == NULL)
    {
        memcpy((void *)&data_reg_array[id], (void *)&reg, sizeof(reg));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
void data_unreg(uint8_t id)
{
    memset((void *)&data_reg_array[id], 0, sizeof(data_reg_t));
    data_reg_array[id].func_ptr = NULL;
}
