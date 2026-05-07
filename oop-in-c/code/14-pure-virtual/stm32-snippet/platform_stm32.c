/* SPDX-License-Identifier: MIT */
/*
 * 本文件是 STM32 真实硬件版本片段。需要 STM32CubeMX 生成的 hi2c1 / htim3 等外设句柄。
 * 实际工程里 platform_xxx 函数体直接调 HAL API。
 * 用 mock-CubeMX 头文件可独立编译验证语法。
 */

/*
 * platform_stm32.c - ch14 STM32 等效片段（必填 + 选填混合）
 *
 * 替换 ch14 pc/ 里 PC 模拟版的 platform 封装函数实现 + 子类实现里
 * 的 printf 模拟. led.c 父类统一接口逻辑 (assert 必填 / 默认行为
 * 选填) / main.c / container_of.h 一字不动.
 *
 * 必填 / 选填 / 全必填三种 ops 表策略不依赖平台. assert 在 STM32 调试
 * 构建里照样把"忘填"暴露给你; Release 构建定义 NDEBUG 后 assert 整行
 * 编译消失. 本章主线讨论的是 led_ops 这一层（子类层）的策略, platform
 * 只是稳定背景, 所以这里直接用 4 个函数把 HAL 包一层即可.
 *
 * GPIO 子类故意只填 on / off (不支持调光), PWM 子类三件套全填 (on /
 * off / set_brightness). 跟 pc/ 一字对齐, 区别只在 printf -> HAL.
 *
 * platform 层从函数式演化成 ops 表是 ch15 的主题.
 */

#include "led.h"
#include "container_of.h"
#include "platform.h"
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

/* ============== GPIO 子类: 只填 on / off, 不填 set_brightness ============== */

static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, self->on_level);
	me->is_on = true;
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, !self->on_level);
	me->is_on = false;
	return 0;
}

/* set_brightness 故意不填: GPIO 不支持调光, 走父类默认行为 (打印"no
 * dimming, skip"). 这是 ch14 选填策略的演示点. */
static const struct led_ops gpio_ops = {
	.on  = gpio_on,
	.off = gpio_off,
};

int led_gpio_init(struct led_gpio *me, const char *name,
                  uint8_t pin, bool on_level)
{
	int rc;
	if (!me)
		return -1;
	rc = led_base_init(&me->base, name, &gpio_ops);
	if (rc != 0)
		return rc;
	me->pin      = pin;
	me->on_level = on_level;
	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
	platform_gpio_write(pin, !on_level);    /* 上电先关灯 */
	return 0;
}

/* ============== PWM 子类: 三件套全填 ==============
 *
 * 走 TIM3 PWM 通道. duty 字段 0-100 (百分比) 转 CCR 值, ARR 假设 1000. */

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
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	uint32_t ch  = pwm_channel_to_hal(self->channel);
	uint32_t ccr = (uint32_t)self->duty * 1000U / 100U;

	__HAL_TIM_SET_COMPARE(&htim3, ch, ccr);
	HAL_TIM_PWM_Start(&htim3, ch);
	me->is_on = true;
	return 0;
}

static int pwm_off(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	uint32_t ch = pwm_channel_to_hal(self->channel);

	__HAL_TIM_SET_COMPARE(&htim3, ch, 0);
	HAL_TIM_PWM_Stop(&htim3, ch);
	me->is_on = false;
	return 0;
}

static int pwm_set_brightness(struct led_base *me, uint8_t brightness)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	uint32_t ch  = pwm_channel_to_hal(self->channel);
	uint32_t ccr;

	if (brightness > 100)
		brightness = 100;
	self->duty = brightness;
	ccr = (uint32_t)brightness * 1000U / 100U;

	__HAL_TIM_SET_COMPARE(&htim3, ch, ccr);
	if (brightness > 0)
		HAL_TIM_PWM_Start(&htim3, ch);
	else
		HAL_TIM_PWM_Stop(&htim3, ch);
	me->is_on = (brightness > 0);
	return 0;
}

static const struct led_ops pwm_ops = {
	.on             = pwm_on,
	.off            = pwm_off,
	.set_brightness = pwm_set_brightness,
};

int led_pwm_init(struct led_pwm *me, const char *name,
                 uint8_t channel, uint8_t duty)
{
	int rc;
	if (!me)
		return -1;
	if (duty > 100)
		return -2;
	rc = led_base_init(&me->base, name, &pwm_ops);
	if (rc != 0)
		return rc;
	me->channel = channel;
	me->duty    = duty;
	return 0;
}
