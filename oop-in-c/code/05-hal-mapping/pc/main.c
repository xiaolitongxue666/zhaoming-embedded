/* SPDX-License-Identifier: MIT */
/*
 * main.c - HAL 库映射验证
 *
 * 这一章不教新概念。只用真实 HAL 命名（GPIO_TypeDef / HAL_GPIO_Init /
 * GPIOA / GPIO_PIN_5）跑一个最小例子，验证 ch01 到 ch04 学的概念
 * 在工业级 HAL 库里全部对应得上。
 */

#include <stdio.h>
#include "hal_gpio.h"

int main(void)
{
	GPIO_InitTypeDef cfg;

	printf("==============================================\n");
	printf("  HAL maps to ch01-ch04 concepts.\n");
	printf("==============================================\n\n");

	printf("--- HAL_GPIO_Init for GPIOA Pin5 (output, push-pull) ---\n");
	cfg.Pin   = GPIO_PIN_5;
	cfg.Mode  = GPIO_MODE_OUTPUT;
	cfg.OType = GPIO_OTYPE_PP;
	cfg.Speed = GPIO_SPEED_HIGH;
	cfg.Pull  = GPIO_PUPD_NONE;
	HAL_GPIO_Init(GPIOA, &cfg);

	printf("\n--- HAL_GPIO_Init for GPIOC Pin13 (board LED) ---\n");
	cfg.Pin   = GPIO_PIN_13;
	cfg.Mode  = GPIO_MODE_OUTPUT;
	cfg.OType = GPIO_OTYPE_PP;
	cfg.Speed = GPIO_SPEED_LOW;
	cfg.Pull  = GPIO_PUPD_NONE;
	HAL_GPIO_Init(GPIOC, &cfg);

	printf("\n--- Same function, different me pointer ---\n");
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, true);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, false);
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

	printf("\n--- ReadPin echos ODR state ---\n");
	HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5);
	HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

	printf("\n--- HAL_GPIO_DeInit (close the door) ---\n");
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_13);

	printf("\n==============================================\n");
	printf("  Mapping table\n");
	printf("    you learned         <-> HAL real name\n");
	printf("    struct led          <-> GPIO_TypeDef\n");
	printf("    red_led, green_led  <-> GPIOA, GPIOB, GPIOC\n");
	printf("    struct led *me      <-> GPIO_TypeDef *GPIOx\n");
	printf("    led_ prefix         <-> HAL_GPIO_ prefix\n");
	printf("    led_init / deinit   <-> HAL_GPIO_Init / DeInit\n");
	printf("    static helpers      <-> static inside hal_gpio.c\n");
	printf("==============================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
