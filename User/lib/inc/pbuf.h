/***
 * @Author:
 * @Date: 2023-04-04 10:06:40
 * @LastEditors: xxx
 * @LastEditTime: 2023-04-04 13:21:27
 * @Description:
 * @email:
 * @Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#ifndef COMPONENTS_COMMON_INCLUDE_PBUF_H_
#define COMPONENTS_COMMON_INCLUDE_PBUF_H_
#include "../inc/data_type_def.h"
#include "../inc/mlist.h"
#include "../inc/malloc.h"

#define PBUF_DBG_EN (1u)
#define PBUF_TYPE_MAX_NUM (3u)

#define PBUF_NUM_MAX (10u)

// 如果size>254,并使用data_analysis接收数据，需要修改data_analysis.c中的DATA_BUF_RECV_SQQ_LEN
#define SMALL_PBUF_BUFFER_SIZE (32)
#define MEDIUM_PBUF_BUFFER_SIZE (32 * 2)
#define LARGE_PBUF_BUFFER_SIZE (32 * 4)

#define SMALL_PBUF_NUM (4u) // 各种PBUF最大个数
#define MEDIUM_PBUF_NUM (4u)
#define LARGE_PBUF_NUM (4u)

#if PBUF_DBG_EN > 0

/*形参*/
#define _PLINE1_ , uint16_t line
#define _PLINE2_ , uint16_t line
/*实参*/
#define __PLINE1 , __LINE__
#define __PLINE2 , __LINE__

#else

#define _PLINE1_
#define _PLINE2_
#define __PLINE1
#define __PLINE2

#endif

enum _PBUF_TYPE
{
    SMALL_PBUF,
    MEDIUM_PBUF,
    LARGE_PBUF,
    PBUF_TYPE_INVALID
};

typedef struct __send_times_t
{
    uint8_t app_send_times;
    uint8_t mac_send_times;
} send_times_t;

typedef struct
{
    int8_t rssi_dbm;
    uint8_t seq;

    nwk_id_t src_id; // 接收到数据帧时，为同步模块提供同步对象信息；
    nwk_id_t dst_id; // 填写帧的目的节点网络地址

    uint8_t send_mode : 2,
        is_ack : 1,
        need_ack : 1,
        crc_ok : 1,
        is_pending : 1,
        debug_info : 1,
        reserved : 1;

    send_times_t already_send_times;
} pkt_attri_t;

typedef struct
{
    struct list_head list;
    uint8_t *data_p;   // 指向数据区
    uint8_t *head;     // 指向数据区的第一个字节
    uint8_t *end;      // 指向数据区的最后一个字节
    uint16_t data_len; // 该pbuf的实际数据长度
    pkt_attri_t attri;
    bool used;
#if PBUF_DBG_EN > 0
    uint16_t alloc_line;
    uint16_t free_line;
#endif
} pbuf_t;

/**
 * pbuf_initz: 为pbuf申请一块内存区域，需要配置各种pbuf的大小和数量等
 */
void pbuf_initz(void);

/**
 * 申请一个pbuf，用来存放用户数据
 *
 * @param size: 用户的数据长度
 * @param _PLINE1_: pbuf_allocz()位置的行号，调用时传入实参形式__PLINE1
 *
 * @return: 申请成功则返回pbuf的指针，失败则进入断言
 */
extern pbuf_t *pbuf_allocz(uint16_t size _PLINE1_);

/**
 * 释放已经使用完的pbuf
 *
 * @param pbuf: 需要操作的pbuf的指针的指针
 * @param _PLINE2_: 调用pbuf_freez()位置的行号，调用时传入实参形式__PLINE2
 *
 * @return: 无
 */
void pbuf_freez(pbuf_t **const pbuf _PLINE2_);

/**
 * 向pbuf->end方向移动pbuf->data_p指针，移动距离为len
 *
 * @param pbuf: 需要操作的pbuf的指针
 * @param len: data_p需要移动的距离
 *
 * @return: 成功则返回data_p指针，失败返回NULL
 */
extern uint8_t *pbuf_skip_datap_forward(pbuf_t *const pbuf,
                                        uint8_t len);

/**
 * 向pbuf->head方向移动pbuf->data_p指针，移动距离为len
 *
 * @param pbuf: 需要操作的pbuf的指针
 * @param len: data_p需要移动的距离
 *
 * @return: 成功则返回data_p指针，失败返回NULL
 */
extern uint8_t *pbuf_skip_datap_backward(pbuf_t *const pbuf,
                                         uint8_t len);

/**
 * 向pbuf的数据区拷贝数据，并移动data_p指针，改变data_len
 *
 * @param pbuf: 目的地址pbuf的指针(从pbuf->data_p开始拷贝)
 * @param src: 源地址的指针
 * @param len: 需要拷贝的数据长度
 *
 * @return: 成功则返回TRUE, 失败则返回FALSE
 */
extern bool pbuf_copy_data_in(pbuf_t *const pbuf,
                              const uint8_t *const src,
                              uint8_t len);

/**
 * 从pbuf的数据区拷贝数据，并移动data_p指针，不改变data_len
 *
 * @param dst: 目的地址的指针
 * @param pbuf: 源地址pbuf的指针(从pbuf->data_p开始拷贝)
 * @param len: 需要拷贝的数据长度
 *
 *  @return: 成功则返回TRUE, 失败则返回
 */
extern bool pbuf_copy_data_out(uint8_t *const dst,
                               pbuf_t *const pbuf,
                               uint8_t len);
#endif /* COMPONENTS_COMMON_INCLUDE_PBUF_H_ */
