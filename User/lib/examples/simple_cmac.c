#include "../inc/data_type_def.h"
#include "../inc/log.h"
#include "../inc/osel_arch.h"
#include "../inc/cmac.h"

static uint8_t key[] = {
    0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
    0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C}; // 密钥
int32_t main(void)
{
    uint8_t *p;
    uint8_t buffer[16] = {0x00};
    uint32_t size = ARRAY_LEN(buffer);
    // 初始化需要校验的数据
    for (int i = 0; i < size; i++)
    {
        buffer[i] = i;
    }
    uint8_t mic[16];            // 存放生成校验数据的数组
    AES_CMAC_CTX AesCmacCtx[1]; // 密钥扩展表
    AES_CMAC_Init(AesCmacCtx);  // 完成密钥扩展表的初始化

    AES_CMAC_SetKey(AesCmacCtx, key); // 完成密钥扩展表数据

    AES_CMAC_Update(AesCmacCtx, buffer, size & 0xFF); // 完成数据的奇偶校验

    AES_CMAC_Final(mic, AesCmacCtx); // 生成16个字节的校验表

    uint32_t xor_vol = (uint32_t)((uint32_t)mic[3] << 24 | (uint32_t)mic[2] << 16 | (uint32_t)mic[1] << 8 | (uint32_t)mic[0]); // 取表4个字节作为校验码

    p = (uint8_t *)&xor_vol;
    LOG_HEX(p, 4); // 打印结果：5c 7e fb 43
}
