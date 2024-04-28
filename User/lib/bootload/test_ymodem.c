#include "unity.h"
#include "ymodem.c"

void setUp(void)
{
    // 这里可以进行每个测试用例开始前的设置
}

void tearDown(void)
{
    // 这里可以进行每个测试用例结束后的清理
}

void test_CRC16(void)
{
    unsigned char data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    TEST_ASSERT_EQUAL_HEX16(EXPECTED_CRC_VALUE, CRC16(data, sizeof(data)));
}

void test_IS_CAP_LETTER(void)
{
    TEST_ASSERT_TRUE(IS_CAP_LETTER('A'));
    TEST_ASSERT_FALSE(IS_CAP_LETTER('a'));
}

void test_IS_LC_LETTER(void)
{
    TEST_ASSERT_TRUE(IS_LC_LETTER('a'));
    TEST_ASSERT_FALSE(IS_LC_LETTER('A'));
}

void test_IS_09(void)
{
    TEST_ASSERT_TRUE(IS_09('0'));
    TEST_ASSERT_FALSE(IS_09('A'));
}

void test_ISVALIDHEX(void)
{
    TEST_ASSERT_TRUE(ISVALIDHEX('A'));
    TEST_ASSERT_TRUE(ISVALIDHEX('a'));
    TEST_ASSERT_TRUE(ISVALIDHEX('0'));
    TEST_ASSERT_FALSE(ISVALIDHEX('G'));
}

void test_ISVALIDDEC(void)
{
    TEST_ASSERT_TRUE(ISVALIDDEC('0'));
    TEST_ASSERT_FALSE(ISVALIDDEC('A'));
}

void test_CONVERTDEC(void)
{
    TEST_ASSERT_EQUAL_HEX8(0, CONVERTDEC('0'));
    TEST_ASSERT_EQUAL_HEX8(9, CONVERTDEC('9'));
}

void test_CONVERTHEX(void)
{
    TEST_ASSERT_EQUAL_HEX8(10, CONVERTHEX('A'));
    TEST_ASSERT_EQUAL_HEX8(10, CONVERTHEX('a'));
    TEST_ASSERT_EQUAL_HEX8(0, CONVERTHEX('0'));
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_CRC16);
    RUN_TEST(test_IS_CAP_LETTER);
    RUN_TEST(test_IS_LC_LETTER);
    RUN_TEST(test_IS_09);
    RUN_TEST(test_ISVALIDHEX);
    RUN_TEST(test_ISVALIDDEC);
    RUN_TEST(test_CONVERTDEC);
    RUN_TEST(test_CONVERTHEX);

    return UNITY_END();
}
