#include "../inc/data_type_def.h"
#include "../inc/log.h"
#include "../inc/osel_arch.h"
#include "../inc/aes.h"

// 全局变量
static aes_context AesContext; // 密钥表
static uint8_t aBlock[] = {0x00, 0x00, 0x00, 0xcc, 0xff, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 数据块
static uint8_t sBlock[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 存放输出结果

int32_t main(void)
{
    uint8_t buf[16] = {0x00};
    uint8_t size = ARRAY_LEN(buf);
    uint8_t key[] = {
        0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
        0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C}; // 密钥

    // 初始化密文
    for (int i = 0; i < size; i++)
    {
        buf[i] = i;
    }

    // 设置预密钥
    osel_memset(AesContext.ksch, 0, ARRAY_LEN(AesContext.ksch));
    aes_set_key(key, 16, &AesContext);

    // 加密
    osel_memcpy(aBlock, buf, size);
    aes_encrypt(aBlock, sBlock, &AesContext);
    LOG_HEX(sBlock, ARRAY_LEN(sBlock)); // 打印加密结果:50 fe 67 cc 99 6d 32 b6 da 09 37 e9 9b af ec 60

    // 解密
    osel_memcpy(aBlock, sBlock, size);
    aes_decrypt(aBlock, sBlock, &AesContext);
    LOG_HEX(sBlock, ARRAY_LEN(sBlock)); // 打印解密结果:00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
}
