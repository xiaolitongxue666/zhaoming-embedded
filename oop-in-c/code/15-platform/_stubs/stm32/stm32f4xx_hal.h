/* Minimal syntax stub for stm32f4xx_hal.h - syntax-check only, NOT for build */
#ifndef STM32F4XX_HAL_H
#define STM32F4XX_HAL_H

#include <stdint.h>

typedef enum { HAL_OK = 0, HAL_ERROR } HAL_StatusTypeDef;
typedef enum { GPIO_PIN_RESET = 0, GPIO_PIN_SET = 1 } GPIO_PinState;

typedef struct {
	uint32_t volatile MODER;
	uint32_t volatile BSRR;
	uint32_t volatile IDR;
	uint32_t volatile ODR;
} GPIO_TypeDef;

typedef struct {
	uint32_t Pin;
	uint32_t Mode;
	uint32_t Pull;
	uint32_t Speed;
} GPIO_InitTypeDef;

typedef struct { int dummy; } TIM_HandleTypeDef;
typedef struct { int dummy; } I2C_HandleTypeDef;

#define GPIOA ((GPIO_TypeDef *)0x40020000UL)
#define GPIOB ((GPIO_TypeDef *)0x40020400UL)
#define GPIOC ((GPIO_TypeDef *)0x40020800UL)
#define GPIOD ((GPIO_TypeDef *)0x40020C00UL)
#define GPIOE ((GPIO_TypeDef *)0x40021000UL)

#define GPIO_MODE_OUTPUT_PP    0x01
#define GPIO_MODE_INPUT        0x00
#define GPIO_NOPULL            0x00
#define GPIO_PULLUP            0x01
#define GPIO_SPEED_FREQ_LOW    0x00

#define TIM_CHANNEL_1   0
#define TIM_CHANNEL_2   1
#define TIM_CHANNEL_3   2
#define TIM_CHANNEL_4   3

#define __HAL_RCC_GPIOA_CLK_ENABLE() do {} while (0)
#define __HAL_RCC_GPIOB_CLK_ENABLE() do {} while (0)
#define __HAL_RCC_GPIOC_CLK_ENABLE() do {} while (0)
#define __HAL_RCC_GPIOD_CLK_ENABLE() do {} while (0)
#define __HAL_RCC_GPIOE_CLK_ENABLE() do {} while (0)

#define __HAL_TIM_SET_COMPARE(htim, ch, ccr) do { (void)(htim); (void)(ch); (void)(ccr); } while (0)

void HAL_GPIO_Init(GPIO_TypeDef *port, GPIO_InitTypeDef *cfg);
void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t mask, GPIO_PinState v);
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *port, uint16_t mask);
HAL_StatusTypeDef HAL_TIM_PWM_Start(TIM_HandleTypeDef *h, uint32_t ch);
HAL_StatusTypeDef HAL_TIM_PWM_Stop(TIM_HandleTypeDef *h, uint32_t ch);
HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *h, uint16_t addr, uint8_t *buf, uint16_t len, uint32_t to);
HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *h, uint16_t addr, uint8_t *buf, uint16_t len, uint32_t to);

#endif
