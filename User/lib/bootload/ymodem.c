/**
 * @file ymodem.c
 * @author xxx
 * @date 2024-02-18 19:32:40
 * @brief
 * 该模块实现了YMODEM协议的核心功能，包括初始化、配置、数据接收、握手、数据传输和结束处理。
 * 它使用了回调函数来处理不同阶段的事件，并使用信号量来同步不同的流程。CRC校验是用来确保数据传输的完整性和准确性
 * @copyright Copyright (c) 2024 by xxx, All Rights Reserved.
 */
#include "ymodem.h"
#include "sys.h"
#include "delay.h"
#include "flow.h"

sqqueue_ctrl_t rym_sqqueue;                   // 一个接收队列控制结构体，用于管理接收到的数据。
static uint32_t tm_sec = 0;                   // 握手超时时间，用于握手阶段的超时计时
static enum rym_stage stage = RYM_STAGE_NONE; // 当前的阶段
static int32_t rym_tm_sec = 0;                // YMODEM超时计时器

static uint8_t aPacketData[_RYM_PKG_SZ]; // 数据包缓冲区

// 回调函数，用于处理不同阶段的事件
static rym_callback rym_on_begin = NULL;
static rym_callback rym_on_data = NULL;
static rym_callback rym_on_end = NULL;
static rym_callback rym_transmit = NULL;

static struct flow handshake_fw; // 握手流程
static struct flow trans_fw;     // 传输流程
static struct flow finsh_fw;     // 结束流程
static struct flow_sem msg_sem;  // 消息信号量，用于同步

static const uint16_t ccitt_table[256] =
    {
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
        0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
        0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
        0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
        0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
        0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
        0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
        0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
        0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
        0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
        0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
        0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
        0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
        0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
        0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
        0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
        0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
        0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
        0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
        0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
        0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
        0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
        0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
        0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
        0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
        0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
        0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
        0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
        0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0};

uint16_t CRC16(unsigned char *q, int len)
{
    uint16_t crc = 0;

    while (len-- > 0)
        crc = (crc << 8) ^ ccitt_table[((crc >> 8) ^ *q++) & 0xff];
    return crc;
}

/* Exported macro ------------------------------------------------------------*/
#define IS_CAP_LETTER(c) (((c) >= 'A') && ((c) <= 'F'))
#define IS_LC_LETTER(c) (((c) >= 'a') && ((c) <= 'f'))
#define IS_09(c) (((c) >= '0') && ((c) <= '9'))
#define ISVALIDHEX(c) (IS_CAP_LETTER(c) || IS_LC_LETTER(c) || IS_09(c))
#define ISVALIDDEC(c) IS_09(c)
#define CONVERTDEC(c) (c - '0')

#define CONVERTHEX_ALPHA(c) (IS_CAP_LETTER(c) ? ((c) - 'A' + 10) : ((c) - 'a' + 10))
#define CONVERTHEX(c) (IS_09(c) ? ((c) - '0') : CONVERTHEX_ALPHA(c))
/**
 * @brief  Convert a string to an integer
 * @param  p_inputstr: The string to be converted
 * @param  p_intnum: The integer value
 * @retval 1: Correct
 *         0: Error
 */
uint32_t Str2Int(uint8_t *p_inputstr, uint32_t *p_intnum)
{
    uint32_t i = 0, res = 0;
    uint32_t val = 0;

    if ((p_inputstr[0] == '0') && ((p_inputstr[1] == 'x') || (p_inputstr[1] == 'X')))
    {
        i = 2;
        while ((i < 11) && (p_inputstr[i] != '\0'))
        {
            if (ISVALIDHEX(p_inputstr[i]))
            {
                val = (val << 4) + CONVERTHEX(p_inputstr[i]);
            }
            else
            {
                /* Return 0, Invalid input */
                res = 0;
                break;
            }
            i++;
        }

        /* valid result */
        if (p_inputstr[i] == '\0')
        {
            *p_intnum = val;
            res = 1;
        }
    }
    else /* max 10-digit decimal input */
    {
        while ((i < 11) && (res != 1))
        {
            if (p_inputstr[i] == '\0')
            {
                *p_intnum = val;
                /* return 1 */
                res = 1;
            }
            else if (((p_inputstr[i] == 'k') || (p_inputstr[i] == 'K')) && (i > 0))
            {
                val = val << 10;
                *p_intnum = val;
                res = 1;
            }
            else if (((p_inputstr[i] == 'm') || (p_inputstr[i] == 'M')) && (i > 0))
            {
                val = val << 20;
                *p_intnum = val;
                res = 1;
            }
            else if (ISVALIDDEC(p_inputstr[i]))
            {
                val = val * 10 + CONVERTDEC(p_inputstr[i]);
            }
            else
            {
                /* return 0, Invalid input */
                res = 0;
                break;
            }
            i++;
        }
    }

    return res;
}

/**
 * @brief Read data from the circular queue.
 *
 * This function reads the specified length of data from the circular queue and stores it in the given buffer.
 * If the length of data in the queue is greater than or equal to the specified length, it directly reads the specified length of data.
 * If the length of data in the queue is less than the specified length, it reads all the data in the queue.
 *
 * @param buffer The buffer to store the data.
 * @param len The length of data to read.
 *
 * @return The actual length of data read.
 */
static uint16_t rym_sqqueue_read(void *buffer, uint16_t len)
{
    uint16_t i = 0;
    uint8_t *buf = buffer;
    if (rym_sqqueue.get_len(&rym_sqqueue) >= len)
    {
        for (i = 0; i < len; i++)
        {
            buf[i] = *((uint8_t *)rym_sqqueue.del(&rym_sqqueue));
        }
    }
    else
    {
        while ((rym_sqqueue.get_len(&rym_sqqueue) != 0) && (i < len))
        {
            buf[i] = *((uint8_t *)rym_sqqueue.del(&rym_sqqueue));
        }
    }

    return i;
}

/**
 * @brief Initializes the parameters for the YMODEM process.
 *
 * This function sets the stage variable to the specified stage and initializes the rym_tm_sec variable with the value of tm_sec.
 * If the stage is RYM_STAGE_ESTABLISHING, it also clears the rym_sqqueue.
 *
 * @param st The stage of the YMODEM process.
 */
static void rym_process_params_init(enum rym_stage st)
{
    stage = st;
    rym_tm_sec = tm_sec;
    if (st == RYM_STAGE_ESTABLISHING)
    {
        rym_sqqueue.clear_sqq(&rym_sqqueue);
    }
}

/**
 * @brief Performs the handshake process for the YMODEM protocol.
 *
 * This function reads packets from the input queue and performs the necessary checks and actions
 * to establish a connection and initiate file transfer using the YMODEM protocol.
 *
 * @param fl The flow structure containing the necessary variables and synchronization mechanisms.
 * @return The result of the handshake process.
 *         - 0: Handshake process completed successfully.
 *         - Non-zero: Handshake process failed.
 */
static uint8_t rym_do_handshake_process(struct flow *fl)
{
    uint8_t index = 0;
    FL_HEAD(fl);
    static uint16_t rym_recv_len = 0;
    static uint16_t recv_crc, cal_crc;
    static uint8_t *file_ptr = NULL;
    static uint8_t file_name[FILE_NAME_LENGTH];
    static uint8_t file_size[FILE_SIZE_LENGTH];
    static uint32_t filesize;
    for (;;)
    {
        FL_LOCK_WAIT_SEM_OR_TIMEOUT(fl, &msg_sem, FL_CLOCK_SEC);
        if (FL_SEM_IS_RELEASE(fl, &msg_sem))
        {
            rym_recv_len = rym_sqqueue_read(&aPacketData[PACKET_START_INDEX], 1);
            if (rym_recv_len == 1)
            {
                if (aPacketData[PACKET_START_INDEX] != RYM_CODE_SOH && aPacketData[PACKET_START_INDEX] != RYM_CODE_STX)
                    continue;

                /* SOH/STX + seq + payload + crc */
                rym_recv_len = rym_sqqueue_read(&aPacketData[PACKET_NUMBER_INDEX],
                                                _RYM_PKG_SZ - 1);
                if (rym_recv_len != (_RYM_PKG_SZ - 1))
                    continue;
                /* sanity check */
                if ((aPacketData[PACKET_NUMBER_INDEX] != 0) || (aPacketData[PACKET_CNUMBER_INDEX] != 0xFF))
                    continue;
                recv_crc = (uint16_t)(*(&aPacketData[PACKET_START_INDEX] + _RYM_PKG_SZ - 2) << 8) |
                           *(&aPacketData[PACKET_START_INDEX] + _RYM_PKG_SZ - 1);
                cal_crc = CRC16(aPacketData + PACKET_DATA_INDEX, _RYM_PKG_SZ - 5);
                if (recv_crc != cal_crc)
                    continue;

                if (rym_on_begin != NULL)
                {
                    file_ptr = aPacketData + PACKET_DATA_INDEX;
                    while ((*file_ptr != 0) && (index < FILE_NAME_LENGTH))
                    {
                        file_name[index++] = *file_ptr++;
                    }
                    file_name[index++] = '\0';
                    index = 0;
                    file_ptr++;
                    while ((*file_ptr != ' ') && (index < FILE_SIZE_LENGTH))
                    {
                        file_size[index++] = *file_ptr++;
                    }
                    file_size[index++] = '\0';
                    Str2Int(file_size, &filesize);

                    if (RYM_CODE_NONE != rym_on_begin(file_name, filesize))
                    {
                        for (uint8_t i = 0; i < RYM_END_SESSION_SEND_CAN_NUM; i++)
                        {
                            aPacketData[0] = RYM_CODE_CAN;
                            rym_transmit(aPacketData, 1);
                        }
                    }
                    else
                    {
                        aPacketData[0] = RYM_CODE_ACK;
                        rym_transmit(aPacketData, 1);
                        FL_LOCK_DELAY(fl, FL_CLOCK_100MSEC * 5);
                        aPacketData[0] = RYM_CODE_C;
                        rym_transmit(aPacketData, 1);
                        rym_process_params_init(RYM_STAGE_TRANSMITTING);
                        FL_LOCK_DELAY(fl, FL_CLOCK_SEC);
                    }
                }
            }
        }
        else
        {
            aPacketData[0] = RYM_CODE_C;
            rym_transmit(aPacketData, 1);
            rym_tm_sec--;
        }
    }
    FL_TAIL(fl);
}

/**
 * @brief Transfers data using the YMODEM protocol.
 *
 * This function is responsible for transferring data using the YMODEM protocol.
 * It receives the size of the data to be transferred and a pointer to the code
 * that will be returned. It performs various checks on the received data and
 * calculates the CRC to ensure data integrity. If all checks pass, it sets the
 * code to RYM_CODE_ACK and returns 0. Otherwise, it returns an error code.
 *
 * @param data_size The size of the data to be transferred.
 * @param code Pointer to the code that will be returned.
 * @return 0 if successful, otherwise an error code.
 */
static int8_t rym_tran_data(uint16_t data_size, uint8_t *code)
{
    DBG_ASSERT(NULL != code __DBG_LINE);
    uint16_t recv_len = 0;
    uint16_t recv_crc, cal_crc;
    const uint16_t tran_size = PACKET_HEADER_SIZE - 1 + data_size + PACKET_TRAILER_SIZE;

    /* seq + data + crc */
    recv_len = rym_sqqueue_read(&aPacketData[PACKET_NUMBER_INDEX],
                                tran_size);
    if (recv_len != tran_size)
        return -RYM_ERR_DSZ;
    /* sanity check */
    if ((aPacketData[PACKET_NUMBER_INDEX] + aPacketData[PACKET_CNUMBER_INDEX]) != 0xFF)
        return -RYM_ERR_SEQ;
    /* As we are sending C continuously, there is a chance that the
     * sender(remote) receive an C after sending the first handshake package.
     * So the sender will interpret it as NAK and re-send the package. So we
     * just ignore it and proceed. */
    if (stage == RYM_STAGE_ESTABLISHED && aPacketData[PACKET_NUMBER_INDEX] == RYM_CODE_NONE)
    {
        *code = RYM_CODE_NONE;
        return 0;
    }

    stage = RYM_STAGE_TRANSMITTING;

    recv_crc = (uint16_t)(*(&aPacketData[PACKET_START_INDEX] + tran_size - 1) << 8) |
               *(&aPacketData[PACKET_START_INDEX] + tran_size);
    cal_crc = CRC16(aPacketData + PACKET_DATA_INDEX, data_size);
    if (recv_crc != cal_crc)
        return -RYM_ERR_CRC;

    *code = RYM_CODE_ACK;
    return 0;
}

/**
 * @brief Performs the YMODEM transmission process.
 *
 * This function is responsible for handling the YMODEM transmission process.
 * It receives packets of data and performs the necessary operations based on the received data.
 * It handles timeouts and retransmissions if necessary.
 *
 * @param fl The flow structure pointer.
 * @return The status of the transmission process.
 */
static uint8_t rym_do_trans_process(struct flow *fl)
{
    FL_HEAD(fl);
    static uint16_t data_size, rym_recv_len;
    static uint8_t rym_code;
    static uint16_t tran_timeout = 0;
    for (;;)
    {
        FL_LOCK_WAIT_SEM_OR_TIMEOUT(fl, &msg_sem, FL_CLOCK_SEC);
        if (FALSE == FL_SEM_IS_RELEASE(fl, &msg_sem))
        {
            if (tran_timeout++ >= 5)
            {
                tran_timeout = 0;
                rym_process_params_init(RYM_STAGE_ESTABLISHING);
                FL_LOCK_DELAY(fl, FL_CLOCK_SEC);
            }
        }
        else
        {
            tran_timeout = 0;
            rym_recv_len = rym_sqqueue_read(&aPacketData[PACKET_START_INDEX], 1);
            if (rym_recv_len == 1)
            {
                if (aPacketData[PACKET_START_INDEX] == RYM_CODE_SOH)
                    data_size = PACKET_SIZE;
                else if (aPacketData[PACKET_START_INDEX] == RYM_CODE_STX)
                    data_size = PACKET_1K_SIZE;
                else if (aPacketData[PACKET_START_INDEX] == RYM_CODE_EOT)
                {
                    aPacketData[0] = RYM_CODE_NAK;
                    rym_transmit(aPacketData, 1);
                    rym_process_params_init(RYM_STAGE_FINISHING);
                    FL_LOCK_DELAY(fl, FL_CLOCK_SEC);
                    continue;
                }
                else
                {
                    rym_process_params_init(RYM_STAGE_ESTABLISHING);
                    FL_LOCK_DELAY(fl, FL_CLOCK_SEC);
                    continue;
                }

                if (rym_tran_data(data_size, &rym_code) == 0)
                {
                    if (rym_on_data != NULL)
                        rym_on_data(aPacketData + PACKET_DATA_INDEX, data_size);

                    if (rym_code == RYM_CODE_CAN)
                    {
                        for (uint8_t i = 0; i < RYM_END_SESSION_SEND_CAN_NUM; i++)
                        {
                            aPacketData[0] = RYM_CODE_CAN;
                            rym_transmit(aPacketData, 1);
                        }
                    }
                    else if (rym_code == RYM_CODE_ACK)
                    {
                        aPacketData[0] = RYM_CODE_ACK;
                        rym_transmit(aPacketData, 1);
                    }
                }
            }
        }
    }
    FL_TAIL(fl);
}

/**
 * @brief Performs the finishing process for the YMODEM protocol.
 *
 * This function is responsible for handling the final stage of the YMODEM protocol,
 * where the receiver receives the end of transmission (EOT) signal from the sender.
 * It verifies the received EOT signal, sends an acknowledgement (ACK) signal back to
 * the sender, and waits for the start of header (SOH) signal to receive the final
 * packet containing the payload and checksum. If the received packet is valid, it
 * calculates the checksum and compares it with the received checksum. If they match,
 * it sets the stage to RYM_STAGE_FINISHED and invokes the callback function if
 * available. This function also handles timeout conditions and reinitializes the
 * protocol parameters if necessary.
 *
 * @param fl The flow structure pointer.
 * @return The result of the finishing process.
 */
static uint8_t rym_do_finish_process(struct flow *fl)
{
    FL_HEAD(fl);
    static uint16_t rym_recv_len;
    static uint16_t recv_crc, cal_crc;
    static uint16_t tran_timeout = 0;

    for (;;)
    {
        FL_LOCK_WAIT_SEM_OR_TIMEOUT(fl, &msg_sem, FL_CLOCK_SEC);
        if (FALSE == FL_SEM_IS_RELEASE(fl, &msg_sem))
        {
            if (tran_timeout++ >= 5)
            {
                tran_timeout = 0;
                rym_process_params_init(RYM_STAGE_ESTABLISHING);
                FL_LOCK_DELAY(fl, FL_CLOCK_SEC);
            }
        }
        else
        {
            tran_timeout = 0;
            /* read the length of the packet */
            rym_recv_len = rym_sqqueue_read(&aPacketData[PACKET_START_INDEX], 1);
            if (rym_recv_len == 1)
            {
                if (aPacketData[PACKET_START_INDEX] != RYM_CODE_EOT)
                    continue;

                /* send an ACK */
                aPacketData[0] = RYM_CODE_ACK;
                rym_transmit(aPacketData, 1);
                FL_LOCK_DELAY(fl, FL_CLOCK_100MSEC * 5);
                /* send a C */
                aPacketData[0] = RYM_CODE_C;
                rym_transmit(aPacketData, 1);

                FL_LOCK_WAIT_SEM_OR_TIMEOUT(fl, &msg_sem, FL_CLOCK_SEC);
                if (FALSE == FL_SEM_IS_RELEASE(fl, &msg_sem))
                    continue;
                /* read the length of the packet */
                rym_recv_len = rym_sqqueue_read(&aPacketData[PACKET_START_INDEX], 1);
                if (rym_recv_len == 1)
                {
                    if (aPacketData[PACKET_START_INDEX] != RYM_CODE_SOH)
                        continue;

                    /* SOH/STX + seq + payload + crc */
                    rym_recv_len = rym_sqqueue_read(&aPacketData[PACKET_NUMBER_INDEX],
                                                    _RYM_SOH_PKG_SZ - 1);
                    if (rym_recv_len != (_RYM_SOH_PKG_SZ - 1))
                        continue;
                    /* sanity check */
                    if ((aPacketData[PACKET_NUMBER_INDEX] != 0) || (aPacketData[PACKET_CNUMBER_INDEX] != 0xFF))
                        continue;
                    recv_crc = (uint16_t)(*(&aPacketData[PACKET_START_INDEX] + _RYM_SOH_PKG_SZ - 2) << 8) |
                               *(&aPacketData[PACKET_START_INDEX] + _RYM_SOH_PKG_SZ - 1);
                    cal_crc = CRC16(aPacketData + PACKET_DATA_INDEX, _RYM_SOH_PKG_SZ - 5);
                    if (recv_crc != cal_crc)
                        continue;

                    /* we got a valid packet. invoke the callback if there is one. */
                    stage = RYM_STAGE_FINISHED;
                    aPacketData[0] = RYM_CODE_ACK;
                    rym_transmit(aPacketData, 1);
                    /* we already got one EOT in the caller. invoke the callback if there is
                     * one. */
                    if (rym_on_end)
                        rym_on_end(&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
                }
            }
        }
    }
    FL_TAIL(fl);
}

/**
 * @brief Process the Ymodem protocol stages.
 *
 * This function is responsible for handling the different stages of the Ymodem protocol.
 * It performs the necessary actions based on the current stage.
 *
 * @note The stages include establishing connection, transmitting data, finishing, and others.
 *
 * @param None
 * @return None
 */
void rym_process(void)
{
    switch (stage)
    {
    case RYM_STAGE_ESTABLISHING: // 建立连接 (Establishing connection)
        rym_do_handshake_process(&handshake_fw);
        break;
    case RYM_STAGE_TRANSMITTING: // 传输中 (Transmitting)
        rym_do_trans_process(&trans_fw);
        break;
    case RYM_STAGE_FINISHING: // 结束 (Finishing)
        rym_do_finish_process(&finsh_fw);
        break;
    case RYM_STAGE_NONE:
        rym_process_params_init(RYM_STAGE_ESTABLISHING);
        break;
    case RYM_STAGE_FINISHED:
        rym_tm_sec = 0;
        break;
    default:
        // rym_process_params_init(RYM_STAGE_ESTABLISHING);
        break;
    }
}

/**
 * @brief Checks if the YMODEM receive timeout has occurred.
 * @return TRUE if the timeout has occurred, FALSE otherwise.
 */
BOOL rym_timeout(void)
{
    return rym_tm_sec == 0;
}

/**
 * @brief Initializes the YMODEM protocol for receiving files.
 *
 * This function initializes the necessary data structures and variables
 * for receiving files using the YMODEM protocol.
 *
 * @return TRUE if initialization is successful, FALSE otherwise.
 */
BOOL rym_init(void)
{
    sqqueue_ctrl_init(&rym_sqqueue, sizeof(uint8_t), _RYM_PKG_SZ);
    FL_INIT(&handshake_fw);
    FL_INIT(&trans_fw);
    FL_INIT(&finsh_fw);
    return TRUE;
}

/**
 * @brief Receive data using the Ymodem protocol.
 *
 * This function is used to receive data transmitted using the Ymodem protocol and store it in the specified buffer.
 *
 * @param p Pointer to the data storage buffer.
 */
uint16_t rym_receive(void *p, uint16_t size)
{
    rym_sqqueue.string_enter(&rym_sqqueue, p, size);
    FLOW_SEM_RELEASE(&msg_sem);
    return 0;
}

/**
 * @brief Configures the YMODEM protocol with the specified callbacks and handshake timeout.
 *
 * This function sets the callback functions for the YMODEM protocol, which will be called during different stages of the protocol.
 * The `on_begin` callback is called when the YMODEM transfer begins.
 * The `on_data` callback is called when a data packet is received during the transfer.
 * The `on_end` callback is called when the YMODEM transfer ends.
 * The `on_transmit` callback is called when a data packet needs to be transmitted during the transfer.
 * The `handshake_timeout` parameter specifies the timeout duration for the handshake phase of the YMODEM protocol.
 *
 * @param on_begin The callback function to be called when the YMODEM transfer begins.
 * @param on_data The callback function to be called when a data packet is received during the transfer.
 * @param on_end The callback function to be called when the YMODEM transfer ends.
 * @param on_transmit The callback function to be called when a data packet needs to be transmitted during the transfer.
 * @param handshake_timeout The timeout duration for the handshake phase of the YMODEM protocol.
 *
 * @return TRUE if the configuration was successful, FALSE otherwise.
 */
BOOL rym_config(rym_callback on_begin, rym_callback on_data,
                rym_callback on_end, rym_callback on_transmit,
                int handshake_timeout)
{
    rym_on_begin = on_begin;
    rym_on_data = on_data;
    rym_on_end = on_end;
    rym_transmit = on_transmit;
    tm_sec = handshake_timeout;
    return TRUE;
}
