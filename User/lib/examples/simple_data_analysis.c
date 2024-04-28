#include "../inc/data_type_def.h"
#include "../inc/log.h"
#include "../inc/osel_arch.h"
#include "../inc/data_analysis.h"
#define UART_RXSIZE (254U)
#define UART_DATA_ANALYSIS_PORT_1 DATA_1
#define UART_DATA_ANALYSIS_PORT_2 DATA_2

static data_interupt_cb_t uart_data_analysis_cb = NULL; // 数据源中断回调函数

static void data_analysis_event1(void)
{
    uint8_t frame[UART_RXSIZE];
    uint8_t data_head[3];
    uint8_t crc[2];
    uint16_t frame_len, out_frame_len;
    data_read(UART_DATA_ANALYSIS_PORT_1, &data_head[0], 3);
    osel_memcpy((uint8_t *)&frame_len, &data_head[1], 2);

    frame_len = B2S_UINT16(frame_len) - 2; // 报文长度包含帧长，这里需要减2
    if (frame_len > UART_RXSIZE)
    {
        lock_data(UART_DATA_ANALYSIS_PORT_1);
        unlock_data(UART_DATA_ANALYSIS_PORT_1);
        return;
    }

    out_frame_len = data_read(UART_DATA_ANALYSIS_PORT_1, frame, (uint16_t)frame_len);
    if (out_frame_len != frame_len)
    {
        return;
    }
    out_frame_len = out_frame_len - 1; // 报文中包含帧尾，这里需要减1

    // 校验CRC_16
    uint16_t crc_16 = 0;
    uint16_t crc16 = crc16_compute(&frame[0], out_frame_len - 2);
    osel_memcpy(&crc[0], &frame[out_frame_len - 2], 2);
    crc_16 = BUILD_UINT16(crc[1], crc[0]);
    if (crc16 != crc_16)
    {
        return;
    }
    // CRC校验通过后将数据长度-2
    out_frame_len -= 2;

    LOG_PRINT("data_analysis_event1 ok:");
    LOG_HEX(frame, out_frame_len);
}

static void data_analysis_event2(void)
{
    uint8_t frame[UART_RXSIZE];
    uint8_t data_head[4];
    uint8_t crc[2];
    uint16_t frame_len, out_frame_len;
    data_read(UART_DATA_ANALYSIS_PORT_2, &data_head[0], 4);
    osel_memcpy((uint8_t *)&frame_len, &data_head[2], 2);
    frame_len = B2S_UINT16(frame_len);
    if (frame_len > UART_RXSIZE)
    {
        lock_data(UART_DATA_ANALYSIS_PORT_2);
        unlock_data(UART_DATA_ANALYSIS_PORT_2);
        return;
    }

    out_frame_len = data_read(UART_DATA_ANALYSIS_PORT_2, frame, (uint16_t)frame_len);
    if (out_frame_len != frame_len)
    {
        return;
    }

    // 校验CRC_16
    uint16_t crc_16 = 0;
    uint16_t crc16 = crc16_compute(&frame[0], out_frame_len - 2);
    osel_memcpy(&crc[0], &frame[out_frame_len - 2], 2);
    crc_16 = BUILD_UINT16(crc[1], crc[0]);
    if (crc16 != crc_16)
    {
        LOG_PRINT("crc error crc16:%x, crc_16:%x\n");
        return;
    }

    out_frame_len -= 2; // 去掉CRC_16

    LOG_PRINT("data_analysis_event2 ok:");
    LOG_HEX(frame, out_frame_len);
}
/**
 * @brief  需要识别帧头和帧尾的数据协议
 * @return {*}
 * @note
 */
static void data_register1(void)
{
/**
 * 帧头	帧长度	源地址	目标地址	报文类型	报文体	    校验	帧尾
    1	2	    2       2            1	        n	     2      1
*/
#define FRAME_HEAD 0x05 // 帧头
#define FRAME_TAIL 0x1b // 帧尾

    // 注册数据解析
    data_reg_t reg;
    reg.sd.valid = true;                                              // 数据头部验证有效标志位
    reg.sd.len = 1;                                                   // 数据头部长度
    reg.sd.pos = 0;                                                   // 数据头部偏移量
    reg.sd.data[0] = FRAME_HEAD;                                      // 数据头部数据
    reg.ld.len = 2;                                                   // 数据长度
    reg.ld.pos = 2;                                                   // 报文长度包含帧长，这里需要设置偏移2
    reg.ld.valid = true;                                              // 数据长度有效标志位
    reg.ld.little_endian = false;                                     // 数据长度是否小端模式
    reg.argu.len_max = UART_RXSIZE;                                   // 数据最大长度
    reg.argu.len_min = 2;                                             // 数据最小长度
    reg.ed.valid = true;                                              // 数据尾部有效标志位
    reg.ed.len = 1;                                                   // 数据尾部长度
    reg.ed.data[0] = FRAME_TAIL;                                      // 数据尾部数据
    reg.echo_en = false;                                              // 是否回显
    reg.func_ptr = data_analysis_event1;                              // 数据解析回调函数   data_analysis模块处理完数据后，会调用这个函数继续数据协议的处理
    uart_data_analysis_cb = data_fsm_init(UART_DATA_ANALYSIS_PORT_1); // 注册数据处理函数   data_analysis模块会调用这个函数，将数据写入到data_analysis模块
    data_reg(UART_DATA_ANALYSIS_PORT_1, reg);                         // 注册数据解析
}

/**
 * @brief  需要识别帧头和没有帧尾的数据协议
 * @return {*}
 * @note
 */
static void data_register2(void)
{
/**
 * 帧头	帧长度	源地址	目标地址	报文类型	报文体	    校验
    2	2	    2       2            1	        n	     2
*/
#define FRAME_HEAD1 0xD5 // 帧头
#define FRAME_HEAD2 0xC8 // 帧尾

    // 注册数据解析
    data_reg_t reg;
    reg.sd.valid = true;                                              // 数据头部验证有效标志位
    reg.sd.len = 2;                                                   // 数据头部长度
    reg.sd.pos = 0;                                                   // 数据头部偏移量
    reg.sd.data[0] = FRAME_HEAD1;                                     // 数据头部数据
    reg.sd.data[1] = FRAME_HEAD2;                                     // 数据头部数据
    reg.ld.len = 2;                                                   // 数据长度
    reg.ld.pos = 2;                                                   // 报文长度包含帧长，这里需要设置偏移2
    reg.ld.valid = true;                                              // 数据长度有效标志位
    reg.ld.little_endian = false;                                     // 数据长度是否小端模式
    reg.argu.len_max = UART_RXSIZE;                                   // 数据最大长度
    reg.argu.len_min = 2;                                             // 数据最小长度
    reg.ed.valid = false;                                             // 数据尾部有效标志位
    reg.echo_en = false;                                              // 是否回显
    reg.func_ptr = data_analysis_event2;                              // 数据解析回调函数   data_analysis模块处理完数据后，会调用这个函数继续数据协议的处理
    uart_data_analysis_cb = data_fsm_init(UART_DATA_ANALYSIS_PORT_2); // 注册数据处理函数   data_analysis模块会调用这个函数，将数据写入到data_analysis模块
    data_reg(UART_DATA_ANALYSIS_PORT_2, reg);                         // 注册数据解析
}

int32_t main(void)
{
    data_register1();
    data_register2();

    // 模拟串口数据
    uint8_t data1[] = {0x05, 0x00, 0x0a, 0xff, 0xff, 0x00, 0x01, 0x00, 0x55, 0x40, 0x1b};
    for (uint16_t i = 0; i < ARRAY_LEN(data1); i++)
    {
        uart_data_analysis_cb(UART_DATA_ANALYSIS_PORT_1, *(data1 + i));
    }

    // 模拟串口数据
    uint8_t data2[] = {0xD5, 0xC8, 0x00, 0x07, 0xff, 0xff, 0x00, 0x01, 0x00, 0x55, 0x40};
    for (uint16_t i = 0; i < ARRAY_LEN(data2); i++)
    {
        uart_data_analysis_cb(UART_DATA_ANALYSIS_PORT_2, *(data2 + i));
    }

    return 0;
}
