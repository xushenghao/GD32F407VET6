/**
 * @file gpios.h
 * @brief Header file for GPIO configuration and control.
 *
 * This file contains the declarations and definitions for GPIO configuration and control functions.
 *
 * @author xxx
 * @date 2023-12-27 14:44:03
 * @version 1.0
 * @copyright Copyright (c) 2024 by xxx, All Rights Reserved.
 */

#ifndef __GPIOS_H__
#define __GPIOS_H__
#include "lib.h"
#include "main.h"

/**
 * @brief Set the GPIO pin to high.
 *
 * @param port The GPIO port.
 * @param pin The GPIO pin.
 */
#define GPIO_SET(port, pin) (HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET))

/**
 * @brief Set the GPIO pin to low.
 *
 * @param port The GPIO port.
 * @param pin The GPIO pin.
 */
#define GPIO_RESET(port, pin) (HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET))

/**
 * @brief Toggle the state of the GPIO pin.
 *
 * @param port The GPIO port.
 * @param pin The GPIO pin.
 */
#define GPIO_TOGGLE(port, pin) (HAL_GPIO_TogglePin(port, pin))

/**
 * @brief Read the state of the GPIO pin.
 *
 * @param port The GPIO port.
 * @param pin The GPIO pin.
 * @return The state of the GPIO pin (1 if high, 0 if low).
 */
#define GPIO_READ(port, pin) (HAL_GPIO_ReadPin(port, pin))

/**
 * @brief Set the GPIO pin as input.
 *
 * @param port The GPIO port.
 * @param pin The GPIO pin.
 */
#define GPIO_SET_INPUT(port, pin) (HAL_GPIO_Init(port, &(GPIO_InitTypeDef){pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_SPEED_FREQ_LOW, 0}))

/**
 * @brief Set the GPIO pin as output.
 *
 * @param port The GPIO port.
 * @param pin The GPIO pin.
 */
#define GPIO_SET_OUTPUT(port, pin) (HAL_GPIO_Init(port, &(GPIO_InitTypeDef){pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0}))

/**
 * @brief Set the GPIO pin as alternate function.
 *
 * @param port The GPIO port.
 * @param pin The GPIO pin.
 */
#define GPIO_SET_ALTERNATE(port, pin) (HAL_GPIO_Init(port, &(GPIO_InitTypeDef){pin, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0}))

/**
 * @brief Set the GPIO pin as analog.
 *
 * @param port The GPIO port.
 * @param pin The GPIO pin.
 */
#define GPIO_SET_ANALOG(port, pin) (HAL_GPIO_Init(port, &(GPIO_InitTypeDef){pin, GPIO_MODE_ANALOG, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0}))

/**
 * @brief Structure representing a GPIO pin.
 */
typedef struct GPIO
{
    GPIO_TypeDef *port; // The GPIO port.
    uint16_t pin;       // The GPIO pin.

    /**
     * @brief Set the GPIO pin to high.
     *
     * @param gpio The GPIO pin.
     */
    void (*set)(struct GPIO gpio);

    /**
     * @brief Set the GPIO pin to low.
     *
     * @param gpio The GPIO pin.
     */
    void (*reset)(struct GPIO gpio);

    /**
     * @brief Toggle the state of the GPIO pin.
     *
     * @param gpio The GPIO pin.
     */
    void (*toggle)(struct GPIO gpio);

    /**
     * @brief Read the state of the GPIO pin.
     *
     * @param gpio The GPIO pin.
     * @return The state of the GPIO pin (1 if high, 0 if low).
     */
    uint8_t (*read)(struct GPIO gpio);
} gpio_t;

/**
 * @brief Create a GPIO pin.
 *
 * @param port The GPIO port.
 * @param pin The GPIO pin.
 * @return The created GPIO pin.
 */
extern gpio_t *gpio_create(GPIO_TypeDef *port, uint16_t pin);

/**
 * @brief Free the memory allocated for a GPIO pin.
 *
 * @param gpio The GPIO pin to free.
 */
extern void gpio_free(gpio_t *gpio);
#endif // __GPIOS_H__
