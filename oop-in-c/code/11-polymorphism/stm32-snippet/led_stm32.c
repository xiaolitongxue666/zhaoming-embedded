/* SPDX-License-Identifier: MIT */
/*
 * 本文件是 STM32 真实硬件版本片段。需要 STM32CubeMX 生成的 htim3 外设句柄。
 * 实际工程里 platform_xxx 函数体直接调 HAL API。
 * 用 mock-CubeMX 头文件可独立编译验证语法。
 */

/*
 * led_stm32.c - 父类统一接口 led_on 落到 STM32 上的样子
 *
 * 父类 led_on / led_off / led_toggle 写在 led_base.c, 子类实现
 * (gpio_on / pwm_on / ...) 直接调 HAL. STM32 上这一层落到
 * HAL_GPIO_xxx / HAL_TIM_PWM_xxx, 应用层 / 父类 / ops 表
 * (.on/.off/.toggle 字段名) 一字不改.
 *
 * 跟 pc/ 唯一的差别就在这个文件: 把 printf 模拟换成真实 HAL 操作.
 */

#include "led.h"
#include "stm32f4xx_hal.h"

/* CubeMX 生成的外设句柄. */
extern TIM_HandleTypeDef htim3;

/* ============== platform 层 (GPIO 封装函数) ============== */

void platform_gpio_init(uint8_t pin, uint8_t mode)
{
	GPIO_InitTypeDef cfg = {0};

	__HAL_RCC_GPIOA_CLK_ENABLE();

	cfg.Pin   = (uint16_t)(1U << pin);
	cfg.Mode  = (mode == GPIO_MODE_OUTPUT) ?
	            GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT;
	cfg.Pull  = GPIO_NOPULL;
	cfg.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &cfg);
}

void platform_gpio_deinit(uint8_t pin)
{
	HAL_GPIO_DeInit(GPIOA, (uint16_t)(1U << pin));
}

void platform_gpio_write(uint8_t pin, bool value)
{
	HAL_GPIO_WritePin(GPIOA, (uint16_t)(1U << pin),
	                  value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool platform_gpio_read(uint8_t pin)
{
	return HAL_GPIO_ReadPin(GPIOA, (uint16_t)(1U << pin)) == GPIO_PIN_SET;
}

/* ============== 子类层: 父类调度过来的子类实现 ============== */

/* ---- GPIO 子类 ---- */

static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	me->is_on = true;
	platform_gpio_write(self->pin, true);
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	me->is_on = false;
	platform_gpio_write(self->pin, false);
	return 0;
}

static int gpio_toggle(struct led_base *me)
{
	if (me->is_on)
		return gpio_off(me);
	return gpio_on(me);
}

/* ---- PWM 子类: 改 TIM3 通道占空比 ---- */

static uint32_t pwm_channel_to_hal(uint8_t channel)
{
	switch (channel) {
	case 1:  return TIM_CHANNEL_1;
	case 2:  return TIM_CHANNEL_2;
	case 3:  return TIM_CHANNEL_3;
	case 4:  return TIM_CHANNEL_4;
	default: return TIM_CHANNEL_1;
	}
}

static int pwm_on(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	uint32_t ch  = pwm_channel_to_hal(self->channel);
	uint32_t ccr = (uint32_t)self->duty * 1000U / 100U;

	__HAL_TIM_SET_COMPARE(&htim3, ch, ccr);
	HAL_TIM_PWM_Start(&htim3, ch);
	me->is_on = true;
	return 0;
}

static int pwm_off(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	uint32_t ch = pwm_channel_to_hal(self->channel);

	__HAL_TIM_SET_COMPARE(&htim3, ch, 0);
	HAL_TIM_PWM_Stop(&htim3, ch);
	me->is_on = false;
	return 0;
}

static int pwm_toggle(struct led_base *me)
{
	if (me->is_on)
		return pwm_off(me);
	return pwm_on(me);
}

/* ============== 两张 ops 表 ============== */

const struct led_ops led_ops_gpio = {
	.on     = gpio_on,
	.off    = gpio_off,
	.toggle = gpio_toggle,
};

const struct led_ops led_ops_pwm = {
	.on     = pwm_on,
	.off    = pwm_off,
	.toggle = pwm_toggle,
};
