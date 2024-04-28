/**
 * @file ymodem.h
 * @author xsh
 * @date 2024-02-18 19:32:46
 * @brief
 * @copyright Copyright (c) 2024 by xxx, All Rights Reserved.
 */
#ifndef __YMODEM_H__
#define __YMODEM_H__

#include "lib.h"

enum rym_code
{
    RYM_CODE_NONE = 0x00,
    RYM_CODE_SOH = 0x01, /* start of 128-byte data packet */
    RYM_CODE_STX = 0x02, /* start of 1024-byte data packet */
    RYM_CODE_EOT = 0x04, /* end of transmission */
    RYM_CODE_ACK = 0x06, /* acknowledge */
    RYM_CODE_NAK = 0x15, /* negative acknowledge */
    RYM_CODE_CAN = 0x18, /* two of these in succession aborts transfer */
    RYM_CODE_C = 0x43,   /* 'C' == 0x43, request 16-bit CRC */
};
typedef enum rym_code rym_code_e;

/* RYM error code
 *
 * We use the rt_err_t to return error values. We take use of current error
 * codes available in RTT and append ourselves.
 */
/* timeout on handshake */
#define RYM_ERR_TMO 0x70
/* wrong code, wrong SOH, STX etc. */
#define RYM_ERR_CODE 0x71
/* wrong sequence number */
#define RYM_ERR_SEQ 0x72
/* wrong CRC checksum */
#define RYM_ERR_CRC 0x73
/* not enough data received */
#define RYM_ERR_DSZ 0x74
/* the transmission is aborted by user */
#define RYM_ERR_CAN 0x75

/* how many ticks wait for chars between packet. */
#ifndef RYM_WAIT_CHR_TICK
#define RYM_WAIT_CHR_TICK (OSEL_TICK_RATE_HZ * 3)
#endif
/* how many ticks wait for between packet. */
#ifndef RYM_WAIT_PKG_TICK
#define RYM_WAIT_PKG_TICK (OSEL_TICK_RATE_HZ * 3)
#endif
/* how many ticks between two handshake code. */
#ifndef RYM_CHD_INTV_TICK
#define RYM_CHD_INTV_TICK (OSEL_TICK_RATE_HZ * 3)
#endif

/* how many CAN be sent when user active end the session. */
#ifndef RYM_END_SESSION_SEND_CAN_NUM
#define RYM_END_SESSION_SEND_CAN_NUM 0x03
#endif

/* Exported constants --------------------------------------------------------*/
/* Packet structure defines */
#define PACKET_HEADER_SIZE ((uint32_t)3)
#define PACKET_DATA_INDEX ((uint32_t)4)
#define PACKET_START_INDEX ((uint32_t)1)
#define PACKET_NUMBER_INDEX ((uint32_t)2)
#define PACKET_CNUMBER_INDEX ((uint32_t)3)
#define PACKET_TRAILER_SIZE ((uint32_t)2)
#define PACKET_OVERHEAD_SIZE (PACKET_HEADER_SIZE + PACKET_TRAILER_SIZE - 1)
#define PACKET_SIZE ((uint32_t)128)
#define PACKET_1K_SIZE ((uint32_t)1024)
#define _RYM_SOH_PKG_SZ (PACKET_SIZE + PACKET_HEADER_SIZE + PACKET_TRAILER_SIZE)
#define _RYM_STX_PKG_SZ (PACKET_1K_SIZE + PACKET_HEADER_SIZE + PACKET_TRAILER_SIZE)
#define _RYM_PKG_SZ _RYM_STX_PKG_SZ // 这里定义的是数据包的大小

/* 因为data是需要写入到flash里面，如果不对齐，会出现UNALIGNED异常
 * /-------- Packet in IAP memory ------------------------------------------\
 * | 0      |  1    |  2     |  3   |  4      | ... | n+4     | n+5  | n+6  |
 * |------------------------------------------------------------------------|
 * | unused | start | number | !num | data[0] | ... | data[n] | crc0 | crc1 |
 * \------------------------------------------------------------------------/
 * the first byte is left unused for memory alignment reasons                 */

#define FILE_NAME_LENGTH ((uint32_t)64)
#define FILE_SIZE_LENGTH ((uint32_t)16)

#define NEGATIVE_BYTE ((uint8_t)0xFF)

#define ABORT1 ((uint8_t)0x41) /* 'A' == 0x41, abort by user */
#define ABORT2 ((uint8_t)0x61) /* 'a' == 0x61, abort by user */

#define MAX_ERRORS ((uint32_t)5)

enum rym_stage
{
    RYM_STAGE_NONE,
    /* set when C is send */
    RYM_STAGE_ESTABLISHING,
    /* set when we've got the packet 0 and sent ACK and second C */
    RYM_STAGE_ESTABLISHED,
    /* set when the sender respond to our second C and recviever got a real
     * data packet. */
    RYM_STAGE_TRANSMITTING,
    /* set when the sender send a EOT */
    RYM_STAGE_FINISHING,
    /* set when transmission is really finished, i.e., after the NAK, C, final
     * NULL packet stuff. */
    RYM_STAGE_FINISHED,
};

/* when receiving files, the buf will be the data received from ymodem protocol
 * and the len is the data size.
 *
 * TODO:
 * When sending files, the len is the buf size in RYM. The callback need to
 * fill the buf with data to send. Returning RYM_CODE_EOT will terminate the
 * transfer and the buf will be discarded. Any other return values will cause
 * the transfer continue.
 */
typedef enum rym_code (*rym_callback)(uint8_t *buf, uint32_t len);

/** recv a file on device dev with ymodem session ctx.
 *
 * If an error happens, you can get where it is failed from ctx->stage.
 *
 * @param on_begin The callback will be invoked when the first packet arrived.
 * This packet often contain file names and the size of the file, if the sender
 * support it. So if you want to save the data to a file, you may need to
 * create the file on need. It is the on_begin's responsibility to parse the
 * data content. The on_begin can be NULL, in which case the transmission will
 * continue without any side-effects.
 *
 * @param on_data The callback will be invoked on the packets received.  The
 * callback should save the data to the destination. The return value will be
 * sent to the sender and in turn, only RYM_{ACK,CAN} is valid. When on_data is
 * NULL, RYM will barely send ACK on every packet and have no side-effects.
 *
 * @param on_end The callback will be invoked when one transmission is
 * finished. The data should be 128 bytes of NULL. You can do some cleaning job
 * in this callback such as closing the file. The return value of this callback
 * is ignored. As above, this parameter can be NULL if you don't need such
 * function.
 *
 * @param handshake_timeout the timeout when hand shaking. The unit is in
 * second.
 */
BOOL rym_config(rym_callback on_begin, rym_callback on_data,
                rym_callback on_end, rym_callback on_transmit,
                int handshake_timeout);

/**
 * @brief Initializes the YMODEM protocol for receiving data.
 *
 * @return BOOL Returns TRUE if initialization is successful, FALSE otherwise.
 */
BOOL rym_init(void);

/**
 * @brief Receives data using the YMODEM protocol.
 *
 * @param p Pointer to the buffer where the received data will be stored.
 * @param size The size of the buffer.
 * @return uint16_t The number of bytes received.
 */
uint16_t rym_receive(void *p, uint16_t size);

/**
 * @brief Processes the received data using the YMODEM protocol.
 */
void rym_process(void);

/**
 * @brief Checks if a timeout has occurred during the YMODEM protocol.
 *
 * @return BOOL Returns TRUE if a timeout has occurred, FALSE otherwise.
 */
BOOL rym_timeout(void);

#endif
