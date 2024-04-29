#include "gpios.h"
#include "gpio.h"
/**
 * @brief 设置GPIO引脚为高电平
 * @param {gpio_t} gpio - GPIO对象
 * @note: 用于设置指定GPIO引脚为高电平。
 */
static void _set(gpio_t gpio)
{
    GPIO_SET(gpio.port, gpio.pin);
}

/**
 * @brief 设置GPIO引脚为低电平
 * @param {gpio_t} gpio - GPIO对象
 * @note: 用于设置指定GPIO引脚为低电平。
 */
static void _reset(gpio_t gpio)
{
    GPIO_RESET(gpio.port, gpio.pin);
}

/**
 * @brief 切换GPIO引脚状态
 * @param {gpio_t} gpio - GPIO对象
 * @note: 用于切换指定GPIO引脚的状态，即高电平变为低电平，低电平变为高电平。
 */
static void _toggle(gpio_t gpio)
{
    GPIO_TOGGLE(gpio.port, gpio.pin);
}

/**
 * @brief 读取GPIO引脚状态
 * @param {gpio_t} gpio - GPIO对象
 * @return {*} - GPIO引脚当前状态，即0表示低电平，1表示高电平
 * @note: 用于读取指定GPIO引脚的状态，即返回0或1。
 */
static uint8_t _read(gpio_t gpio)
{
    return (uint8_t)GPIO_READ(gpio.port, gpio.pin);
}

/**
 * @brief 创建GPIO对象
 * @param {GPIO_TypeDef} *port - GPIO寄存器指针
 * @param {uint16_t} pin - 引脚号
 * @return {gpio_t *} - 创建的GPIO对象指针
 * @note: 用于创建一个GPIO对象，用于操作特定端口和引脚的GPIO功能。
 */
gpio_t *gpio_create(GPIO_TypeDef *port, uint16_t pin)
{
    gpio_t *gpio = (gpio_t *)osel_mem_alloc(sizeof(gpio_t));
    DBG_ASSERT(gpio != NULL __DBG_LINE);
    gpio->port = port;
    gpio->pin = pin;
    gpio->set = _set;
    gpio->reset = _reset;
    gpio->toggle = _toggle;
    gpio->read = _read;
    return gpio;
}

/**
 * @brief 释放GPIO对象
 * @param {gpio_t} *gpio - GPIO对象指针
 * @return {*}
 * @note: 用于释放一个GPIO对象，释放后不能再使用该对象。
 */
void gpio_free(gpio_t *gpio)
{
    if (gpio != NULL)
    {
        osel_mem_free(gpio);
    }
}
